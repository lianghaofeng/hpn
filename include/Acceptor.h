#pragma once

#include "Channel.h"
#include "InetAddress.h"
#include "Socket.h"
#include <functional>

class EventLoop;

class Acceptor {
  public:
    using NewConnectionCallback =
        std::function<void(int sockfd, const InetAddress &)>;
    Acceptor(EventLoop *loop, const InetAddress &listenAddr);

    void setNewConnectionCallback(NewConnectionCallback cb);
    void listen();
    bool listening() const;

  private:
    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};