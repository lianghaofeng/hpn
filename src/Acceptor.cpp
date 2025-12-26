#include "Acceptor.h"
#include "EventLoop.h"
#include "Logger.h"
#include <unistd.h>

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr):
    loop_(loop), 
    acceptSocket_(Socket::createTCP().value()), 
    acceptChannel_(loop, acceptSocket_.fd)()),
    listening_(false){
    
    // 1. socket设置
    acceptSocket_.setReuseAddr();
    acceptSocket_.setNonBlocking();

    // 2.绑定地址
    if(!acceptSocket_.bind(listenAddr)){
        LOG_ERROR("Acceptor bind failed");
    }

    // 3. 设置Channel的读回调
    acceptChannel_.setReadCallback([this](){
        handleRead();
    });
}

void Acceptor::setNewConnectionCallback(NewConnectionCallback cb){
    
}
