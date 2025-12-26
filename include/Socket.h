#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <vector>

class Socket {
public:
    static std::optional<Socket> createTCP();
    
    explicit Socket(int fd);

    // Construct from existing fd
    Socket(Socket&& other) noexcept;
    Socket& operator = (Socket&& other) noexcept;

    // Non-copyable
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    ~Socket();

    bool bind(uint16_t port_);

    bool listen(int backlog =128);

    std::optional<Socket> accept();

    bool setNonBlocking();

    bool setReuseAddr();

    int fd() const {return fd_; }

    bool isValid() const { return fd_ >= 0; }

    std::string getLastError() const;

private:

    int fd_;

    static std::string getSystemError();
    
};