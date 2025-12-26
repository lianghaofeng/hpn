#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop* loop, int fd):
    loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0) {
}

Channel::~Channel() {
    // 因为没有fd，所以不需要关闭
}

void Channel::handleEvent(){
    // 处理挂断，同时在有数据可读时不触发close
    if((revents_& EPOLLHUP) && !(revents_ & EPOLLIN)){
        
        if(closeCallback_){
            closeCallback_();
        }
    }

    // 处理错误
    if(revents_ & EPOLLERR) {
        if (errorCallback_) {
            errorCallback_();
        }
    }

    // 读
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if(readCallback_){
            readCallback_();
        }
    }

    // 写
    if(revents_ & EPOLLOUT) {
        if(writeCallback_){
            writeCallback_();
        }
    }
}

void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::remove() {
    loop_->removeChannel(this);
}