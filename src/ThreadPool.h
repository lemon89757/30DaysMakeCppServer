#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

class ThreadPool{
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex task_mutex;
    std::condition_variable cv;
    bool stop;
public:
    ThreadPool(int size = 10);      // 默认 size 最好设置为 std::thread::hardware_concurrency()
    ~ThreadPool();

    template<class F, class... Args>
    auto add(F&& f, Args&&... args) -> 
    std::future<typename std::result_of<F(Args...)>::type>;
};

// 不能放在 cpp 文件，原因是 C++ 编译器不支持模版的分离编译
template<class F, class...Args>
auto ThreadPool::add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(task_mutex);
        // don't allow enqueueing after stopping the pool
        if(stop) throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){(*task)();});
    }

    cv.notify_one();
    return res;
}