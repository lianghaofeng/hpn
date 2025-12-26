#pragma once

#include "TcpConnection.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include <map>
#include <memory>
#include <string>

class EventLoop;

class TcpServer {
  public:
    TcpServer(EventLoop *loop, const InetAddress &listenAddr);
    void start();

    void setConnectionCallback(const ConnectionCallback &cb);
    void setMessageCallback(const MessageCallback &cb);
    void setWriteCompleteCallback(const WriteCompleteCallback &cb);

  private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);

    EventLoop *loop_;
    std::unique_ptr<Acceptor> acceptor_;
    std::map<std::string, TcpConnectionPtr> connections_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    int nextConnId_;
};