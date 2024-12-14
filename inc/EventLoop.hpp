#pragma once

#include <thread>
#include "Epoll.hpp"
#include "pools/ThreadPool.hpp"

class EventLoop : public Epoll
{
public:
    EventLoop() : Epoll() {
        threadWorkers = new ThreadPool(std::thread::hardware_concurrency());
    }
    ~EventLoop();

    /**
     * @brief 开启Reactor循环
     * @param timeout epoll_wait等待时间 
     */
    void start(int timeout=-1);
private:
    ThreadPool *threadWorkers;    
};