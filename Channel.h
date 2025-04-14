#pragma once
#include <sys/epoll.h>

class Epoll;

/* 通过 Channel 类来包含所监听文件描述符的具体事件
 * 从而在通过 epoll 获取到文件描述符以外，还能够得到对应文件描述符所监测的事件
 */
class Channel{
private:
    Epoll* ep;          // 被监听文件描述符所对应的 Epoll 实例
    int fd;             // 被监听的文件描述符
    uint32_t events;    // 表示希望监听该文件描述符的哪些事件
    uint32_t revents;   // 表示在 epoll 返回该 Channel 时文件描述符正在发生的事件
    bool inEpoll;       // 标志位，是否已被 epoll 对象监测

public:
    Channel(Epoll *_ep, int fd);
    ~Channel();

    void enableReading();
    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();
    void setRevents(uint32_t);
    bool getInEpoll();
    void setInEpoll();
};