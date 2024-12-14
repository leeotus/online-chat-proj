#include "EventLoop.hpp"
#include "Connection.hpp"
#include "pools/ThreadPool.hpp"

#include <unistd.h>
#include <memory>

EventLoop::~EventLoop()
{
    delete threadWorkers;
}

// BUG: 因为多线程使用的是shared_ptr,在执行完成之后会对Connection指针进行释放
// 但是exeSendCallback最后有ACTION_DELETE操作,会对Connection指针二次释放内存
// 从而导致错误发生.
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
