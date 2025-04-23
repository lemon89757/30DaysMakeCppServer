#include "ThreadPool.h"

ThreadPool::ThreadPool(int size) : stop(false){
    for(int i = 0; i < size; i++){
        // 启动 size 个线程
        threads.emplace_back(std::thread([this](){
            // 定义每个线程的工作函数
            while(true){
                std::function<void()> task;
                {
                    // 在 {} 作用域内对 std::mutex 加锁，出了作用域会自动解锁，不需要调用 unlock()
                    std::unique_lock<std::mutex> lock(task_mutex);
                    cv.wait(lock, [this](){
                        // 等待条件变量，条件为任务队列不为空或线程池停止
                        return stop || !tasks.empty();
                    });
                    // 任务队列为空且线程池停止，退出线程
                    if(stop && tasks.empty()) return;
                    // 从任务队列头取出任务
                    task = tasks.front();
                    tasks.pop();
                }
                // 释放锁并执行任务
                task();
            }
        }));
    }
}

ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(task_mutex);
        stop = true;
    }
    // 在析构线程池之前先执行完任务队列中的任务
    cv.notify_all();
    for(std::thread& th : threads){
        // 在析构函数中，需要确保所有的线程都正常结束。
        // 在尝试 join 线程之前，需要先检查该线程是否可以被 join，以避免对无效线程调用 join 导致未定义行为
        // 如果线程 th 是有效的，即它代表了一个正在运行的线程，且该线程尚未被 join 或 detach，则 th.joinable() 返回 true
        // th.join() 等待线程 th 完成其执行。当调用 th.join() 时，当前线程会阻塞，直到 th 执行完成
        // 在线程池的析构函数中，调用 th.join() 是为了确保线程池中的所有线程都正常结束，避免程序在析构函数返回后意外终止，
        // 导致线程池中的线程可能还在运行，从而引发资源泄露或其他未定义行为
        if(th.joinable()) th.join();
    }
}

/* 线程池只能接受 std::function<void()> 类型参数， 
 * 所以函数参数需要事先使用 std::bind()，并且无法得到返回值
 */
void ThreadPool::add(std::function<void()> func){
    {
        std::unique_lock<std::mutex> lock(task_mutex);
        if(stop){
            throw std::runtime_error("ThreadPool already stop, can't add task any more");
        }
        tasks.emplace(func);
    }
    // 通知一次条件变量
    cv.notify_one();
}