#pragma once

#include <vector>
#include <map>
#include <sys/epoll.h>

// 前向说明
class Channel;

class EventLoop{
public:
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator = (const EventLoop&) = delete;

    // 开始循环
    void loop();

    // 退出循环
    void quit();

    void updateChannel(Channel* channel);

    void removeChannel(Channel* channel);

private:
    using ChannelMap = std::map<int, Channel*>;

    bool looping_;
    bool quit_;
    int epollfd_;

    std::vector<struct epoll_event> events_;
    ChannelMap channels_;

    static const int kMaxEvents = 16;

};