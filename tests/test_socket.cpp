#include "../include/Socket.h"
#include <cassert>
#include <iostream>
#include <cstring>
#include <unistd.h>

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running" #name "..."; \
    test_##name(); \
    std::cout << "PASSED" << std::endl;\
} while(0)

TEST(create_tcp_socket) {
    auto socket = Socket::createTCP();
    assert(socket.has_value());
    assert(socket->isValid());
    assert(socket->fd() >= 0);
}

TEST(socket_raii){
    int fd;
    {
        auto socket1 = Socket::createTCP();
        assert(socket1.has_value());
        fd = socket1->fd();
        // 出作用域后，socket 会被关闭
    }
    // fd被关闭，read应该失败
    char buf[1];
    ssize_t result = read(fd, buf, 1);
    assert(result < 0);

}


TEST(socket_move) {
    auto socket1 = Socket::createTCP();
    assert(socket1.has_value());
    int original_fd = socket1->fd();

    Socket socket2 = std::move(*socket1);
    assert(socket2.isValid());
    assert(socket2.fd() == original_fd);
    assert(!socket1->isValid());
}

TEST(bind_and_listen) {
    auto socket = Socket::createTCP();
    assert(socket.has_value());

    assert(socket->setReuseAddr());

    assert(socket->bind(9999));

    assert(socket->listen());

}

TEST(set_nonblocking) {
    auto socket = Socket::createTCP();

    assert(socket.has_value());

    assert(socket->setNonBlocking());

    auto accepted = socket->accept();

    assert(!accepted.has_value());
}

TEST(multiple_sockets){
    auto socket1 = Socket::createTCP();
    auto socket2 = Socket::createTCP();

    assert(socket1.has_value());
    assert(socket2.has_value());
    assert(socket1->fd() != socket2->fd());
}

int main() {
    std::cout << "=== Socket Tests ===" << std::endl;
    
    RUN_TEST(create_tcp_socket);
    RUN_TEST(socket_raii);
    RUN_TEST(socket_move);
    RUN_TEST(bind_and_listen);
    RUN_TEST(set_nonblocking);
    RUN_TEST(multiple_sockets);
    
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}