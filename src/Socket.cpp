#include "Socket.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>

std::optional<Socket> Socket::createTCP(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0){
        return std::nullopt;
    }
    return Socket(fd);
}

Socket::Socket(int fd) : fd_(fd) {
}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if(this != &other) {
        if(fd_ >= 0){
            close(fd_);
        }
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

Socket::~Socket() {
    if (fd_ >= 0){
        close(fd_);
    }
}

bool Socket::bind(uint16_t port){
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int result = ::bind(fd_, (struct sockaddr*) &addr, sizeof(addr));
    return result == 0;
}

bool Socket::listen(int backlog){
    int result = ::listen(fd_, backlog);
    return result == 0;
}

std::optional<Socket> Socket::accept() {

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    int conn_fd = ::accept(fd_, (struct sockaddr*)&addr, &len);
    if (conn_fd < 0){
        return std::nullopt;
    }

    return Socket(conn_fd);
}

bool Socket::setNonBlocking() {
    int flags = fcntl(fd_, F_GETFL, 0);
    if(flags < 0) {
        return false;
    }

    flags |= O_NONBLOCK;
    int result = fcntl(fd_, F_SETFL, flags);

    return result >= 0;
}

bool Socket::setReuseAddr() {
    int optval = 1;
    int result = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, 
                            &optval, sizeof(optval));
    return result == 0;
}

std::string Socket::getLastError() const {
    return getSystemError();
}

std::string Socket::getSystemError() {
    return strerror(errno);
}