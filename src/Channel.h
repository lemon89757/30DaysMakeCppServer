#pragma once
#include <sys/epoll.h>
#include <functional>

class EventLoop;

/* 通过 Channel 类来包含所监听文件描述符的具体事件
 * 从而在通过 epoll 获取到文件描述符以外，还能够得到对应文件描述符所监测的事件
 */
class Channel{
private:
    EventLoop* loop;    
    int fd;             // 被监听的文件描述符
    uint32_t events;    // 表示希望监听该文件描述符的哪些事件
    uint32_t revents;   // 表示在 epoll 返回该 Channel 时文件描述符正在发生的事件
    bool inEpoll;       // 标志位，是否已被 epoll 对象监测
    // std::function 是一个泛型函数封装器，可以存储、调用和管理任何可调用对象
    // 例如普通函数、Lambda 表达式、函数对象等
    std::function<void()> callback;

public:
    Channel(EventLoop*_loop, int _fd);
    ~Channel();

    void handleEvent();
    void enableReading();

    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();
    bool getInEpoll();
    void setInEpoll();

    void setRevents(uint32_t);
    void setCallback(std::function<void()>);
};