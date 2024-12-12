#pragma once

#include "Epoll.hpp"

class EventLoop : public Epoll
{
public:
    EventLoop() : Epoll() {}
    ~EventLoop();

    /**
     * @brief 开启Reactor循环
     * @param timeout epoll_wait等待时间 
     */
    void start(int timeout=-1);

};