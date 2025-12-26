#pragma once

#include "Buffer.h"
#include "Channel.h"
#include "Socket.h"
#include <functional>
#include <memory>

class EventLoop;

/**
 * TCP 设计
 * - 拥有Socket
 * - 拥有Channel
 * - 管理读写缓冲区
 * - 提供高层回调接口
 * - 使用 shared_ptr 管理生命期
 *
 */
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
  public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
    using MessageCallback =
        std::function<void(const TcpConnectionPtr &, Buffer *)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr &)>;

    enum State { kConnecting, kConnected, kDisconnecting, kDisconnected };

    TcpConnection(EventLoop *loop, Socket &&socket);
    ~TcpConnection();

    // 删除拷贝构造函数和拷贝赋值运算符
    TcpConnection(const TcpConnection &) = delete;
    TcpConnection &operator=(const TcpConnection &) = delete;

    void setConnectionCallback(ConnectionCallback cb) {
        connectionCallback_ = std::move(cb);
    }

    void setMessageCallback(MessageCallback cb) {
        messageCallback_ = std::move(cb);
    }

    void setCloseCallback(CloseCallback cb) { closeCallback_ = std::move(cb); }

    // TcpServer调用，标记连接已建立
    void connectEstablished();

    // TcpServer调用，连接销毁前的清理
    void connectDestroyed();

    void send(const std::string &message);
    void send(const char *data, size_t len);

    void shutdown();

    State state() const { return state_; }
    bool connected() const { return state_ == kConnected; }

    EventLoop *getLoop() const { return loop_; }

  private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string &message);
    void shutdownInLoop();
    void setState(State s) { state_ = s; }

    EventLoop *loop_;
    Socket socket_;
    std::unique_ptr<Channel> channel_;

    State state_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
};