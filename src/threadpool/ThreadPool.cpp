#include "threadpool/ThreadPool.hpp"
#include <functional>

ThreadPool::ThreadPool(int _thread_num) : quit(false)
{
    if(_thread_num <=0 || _thread_num > std::thread::hardware_concurrency())
    {
        _thread_num = std::thread::hardware_concurrency();
    }
    thread_num = _thread_num;
    
    for(int i=0;i<thread_num;++i)
    {
        workers.emplace_back([this](){
            while(true)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> _g(mtx);
                    cv.wait(_g, [this](){
                        return (quit || !tasks.empty());
                    });
                    if(quit && tasks.empty())
                    {
                        return;
                    }
                    task = tasks.front();
                    tasks.pop();
                }
                // 执行任务
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> _g(mtx);
        quit = true;
        cv.notify_all();
    }

    for(auto &t : workers)
    {
        if(t.joinable())
        {
            t.join();
        }
    }
}