#include "EventLoop.h"
#include "Channel.h"
#include <unistd.h>
#include <cstring>
#include <cassert>

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    epollfd_(epoll_create1(EPOLL_CLOEXEC)),
    events_(kMaxEvents){
        assert(epollfd_);
}

EventLoop::~EventLoop(){
    if(epollfd_ >= 0){
        close(epollfd_);
    } 
}

void EventLoop::loop(){
    assert(!looping_);
    looping_ = true;
    quit_ = false;

    while(!quit_){
        int numEvents = epoll_wait(epollfd_, events_.data(),
                                    static_cast<int>(events_.size()), -1);
        if (numEvents < 0) {
            break;
        }

        for (int i = 0; i < numEvents; ++i) {
            Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
            channel->setRevents(events_[i].events); //设置事件返回类型
            channel->handleEvent();
        }
    }

    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
}

void EventLoop::updateChannel(Channel* channel){
    int fd = channel->fd();

    struct epoll_event event{};
    event.events = channel->events();
    event.data.ptr = channel;

    if(channels_.find(fd) == channels_.end()){
        channels_[fd] = channel;
        epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event);
    } else {
        if (channel->isNoneEvent()){
            epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &event);
        } else {
            epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event);
        }
    }
}

void EventLoop::removeChannel(Channel* channel) {
    int fd = channel->fd();

    auto it = channels_.find(fd);
    if(it != channels_.end()){
        channels_.erase(it);
        epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, nullptr);
    }
}