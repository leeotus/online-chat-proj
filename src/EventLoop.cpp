#include "EventLoop.hpp"
#include "Connection.hpp"
#include "pools/ThreadPool.hpp"

#include <unistd.h>
#include <memory>

EventLoop::~EventLoop()
{
    delete threadWorkers;
}

void dealConnection(Connection *c)
{
    if(c->getREvent()->events & EPOLLIN)
    {
        c->exeRecvCallback();
    } else if(c->getREvent()->events & EPOLLOUT)
    {
        c->exeSendCallback();
    }
}

void EventLoop::start(int timeout)
{
    while(true)
    {
        auto readyConns = wait(timeout);
        for(auto c : readyConns)
        {
            threadWorkers->enqueue(dealConnection, c);
        }
    }
}
