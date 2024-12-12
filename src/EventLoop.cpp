#include "EventLoop.hpp"
#include "Connection.hpp"
#include <unistd.h>

EventLoop::~EventLoop()
{
}

void EventLoop::start(int timeout)
{
    auto readyConns = wait(timeout);
    for(auto *c : readyConns)
    {
        if(c->getREvent()->events & EPOLLIN)
        {
            c->exeRecvCallback();
        } else if(c->getREvent()->events & EPOLLOUT)
        {
            c->exeSendCallback();
        }
    }
}
