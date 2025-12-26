#include "../include/EventLoop.h"
#include "../include/Socket.h"
#include "../include/TcpConnection.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

#define TEST(name) void name()
#define RUN_TEST(name)                                                         \
    do {                                                                       \
        std::cout << "Running " << #name << "...";                             \
        name();                                                                \
        std::cout << " PASSED" << std::endl;                                   \
    } while (0)

// 测试 1: 创建和销毁 TcpConnection
TEST(test_tcpconnection_create) {
    EventLoop loop;

    // 创建 socketpair
    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    Socket sock(fds[0]);
    sock.setNonBlocking();

    // 创建 TcpConnection
    auto conn = std::make_shared<TcpConnection>(&loop, std::move(sock));

    assert(conn->state() == TcpConnection::kConnecting);
    assert(!conn->connected());

    close(fds[1]);
}

// 测试 2: 连接建立
TEST(test_tcpconnection_establish) {
    EventLoop loop;

    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    Socket sock(fds[0]);
    sock.setNonBlocking();

    auto conn = std::make_shared<TcpConnection>(&loop, std::move(sock));

    bool connectionEstablished = false;
    conn->setConnectionCallback([&](const TcpConnection::TcpConnectionPtr &) {
        connectionEstablished = true;
    });

    conn->connectEstablished();

    assert(connectionEstablished);
    assert(conn->state() == TcpConnection::kConnected);
    assert(conn->connected());

    conn->connectDestroyed();
    close(fds[1]);
}

// 测试 3: 发送和接收数据
TEST(test_tcpconnection_send_receive) {
    EventLoop loop;

    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    Socket sock(fds[0]);
    sock.setNonBlocking();

    auto conn = std::make_shared<TcpConnection>(&loop, std::move(sock));

    std::string received;
    conn->setMessageCallback(
        [&](const TcpConnection::TcpConnectionPtr &, Buffer *buf) {
            received = buf->retrieveAllAsString();
            loop.quit();
        });

    conn->connectEstablished();
    


    // 从另一端写入数据
    const char *msg = "hello world";
    ssize_t n = write(fds[1], msg, strlen(msg));
    assert(n == static_cast<ssize_t>(strlen(msg)));

    // 运行事件循环等待读取
    loop.loop();

    assert(received == msg);

    conn->connectDestroyed();
    close(fds[1]);
}

// 测试 4: TcpConnection 发送数据
TEST(test_tcpconnection_send) {
    EventLoop loop;

    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    Socket sock(fds[0]);
    sock.setNonBlocking();

    auto conn = std::make_shared<TcpConnection>(&loop, std::move(sock));

    conn->connectEstablished();

    // 从 TcpConnection 发送数据
    const char *msg = "test message";
    conn->send(msg);

    // 从另一端读取
    char buf[128] = {0};
    ssize_t n = read(fds[1], buf, sizeof(buf));
    assert(n == static_cast<ssize_t>(strlen(msg)));
    assert(strcmp(buf, msg) == 0);

    conn->connectDestroyed();
    close(fds[1]);
}

// 测试 5: 大数据发送（测试输出缓冲区）
TEST(test_tcpconnection_large_send) {
    EventLoop loop;

    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    Socket sock(fds[0]);
    sock.setNonBlocking();

    auto conn = std::make_shared<TcpConnection>(&loop, std::move(sock));

    conn->connectEstablished();

    // 发送大量数据
    std::string largeData(10240, 'x'); // 10KB 数据
    conn->send(largeData);

    // 分批读取
    std::string received;
    char buf[1024];
    while (received.size() < largeData.size()) {
        ssize_t n = read(fds[1], buf, sizeof(buf));
        if (n > 0) {
            received.append(buf, n);
        } else if (n == 0) {
            break;
        } else if (errno == EAGAIN) {
            // 需要等待更多数据
            usleep(1000);
            continue;
        } else {
            break;
        }
    }

    assert(received == largeData);

    conn->connectDestroyed();
    close(fds[1]);
}

// 测试 6: 连接关闭
TEST(test_tcpconnection_close) {
    EventLoop loop;

    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    Socket sock(fds[0]);
    sock.setNonBlocking();

    auto conn = std::make_shared<TcpConnection>(&loop, std::move(sock));

    bool closeCalled = false;
    // conn->setCloseCallback([&](const TcpConnection::TcpConnectionPtr &c) {
    //     closeCalled = true;
    //     std::cout << "closeCalled: " << closeCalled << '/';
    //     loop.quit();
    // });

    conn->setConnectionCallback([&](const TcpConnection::TcpConnectionPtr & c){
        if(!c->connected()){
            closeCalled = true;
            std::cout<<"-关闭连接-";
            loop.quit();
        }
    });

    conn->connectEstablished();

    // 关闭对端
    close(fds[1]);

    // 运行事件循环等待关闭事件
    loop.loop();

    assert(closeCalled);
    assert(conn->state() == TcpConnection::kDisconnected);

    conn->connectDestroyed();
}

// 测试 7: 回调触发
TEST(test_tcpconnection_callbacks) {
    EventLoop loop;

    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    Socket sock(fds[0]);
    sock.setNonBlocking();

    auto conn = std::make_shared<TcpConnection>(&loop, std::move(sock));

    int callbackCount = 0;

    conn->setConnectionCallback([&](const TcpConnection::TcpConnectionPtr &c) {
        if (c->connected()) {
            callbackCount++;
        }
    });

    conn->setMessageCallback(
        [&](const TcpConnection::TcpConnectionPtr &, Buffer *buf) {
            callbackCount++;
            buf->retrieveAll();
            loop.quit();
        });

    conn->connectEstablished();
    assert(callbackCount ==
           1); // connectEstablished 应该触发 connectionCallback

    // 发送数据
    const char *msg = "test";
    write(fds[1], msg, strlen(msg));

    // 运行事件循环
    loop.loop();

    assert(callbackCount == 2); // 应该触发 messageCallback

    conn->connectDestroyed();
    close(fds[1]);
}

int main() {
    RUN_TEST(test_tcpconnection_create);
    RUN_TEST(test_tcpconnection_establish);
    RUN_TEST(test_tcpconnection_send_receive);
    RUN_TEST(test_tcpconnection_send);
    RUN_TEST(test_tcpconnection_large_send);
    RUN_TEST(test_tcpconnection_close);
    RUN_TEST(test_tcpconnection_callbacks);

    std::cout << "\n=== All TcpConnection Tests Passed ===" << std::endl;
    return 0;
}
