#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <vector>
#include <queue>
#include <memory>

class ThreadPool
{
public:
    /**
     * @brief Construct a new Thread Pool object
     * 线程池构造函数
     * @param _thread_num 线程池内要启动的线程数量 
     * @note 默认参数值为0，表示启动当前系统所能使用的最大线程数供给线程池
     */
    ThreadPool(int _thread_num=0);

    ~ThreadPool();

    /**
     * @brief 推送任务到线程池并通知线程池开始工作
     * @tparam F 输入的任务函数类型 
     * @tparam Args 任务函数的参数类型
     * @param f 输入的函数指针
     * @param args 要传入到函数f指针的参数parameter pack
     * @return std::future<typename std::result_of<F(Args...)>::type> 
     * 任务完成之后可以通过future来获取结果
     */
    template<typename F, typename ...Args>
    auto enqueue(F&& f, Args&& ...args) -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    int thread_num;
    bool quit;
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
};


template<typename F, typename ...Args>
auto ThreadPool::enqueue(F&& f, Args&& ...args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using returnType = typename std::result_of<F(Args...)>::type;
    std::future<returnType> res;

    auto taskPtr = std::make_shared<std::packaged_task<returnType()>>(std::bind(
        std::forward<F>(f), std::forward<Args>(args)...
    ));
    res = taskPtr.get_future();

    {
        std::unique_lock<std::mutex> _g(mtx);
        tasks.emplace([taskPtr]() {
            (*taskPtr)();
        });
        // 通知一个线程开始工作
        cv.notify_one();
    }

    return res;
}