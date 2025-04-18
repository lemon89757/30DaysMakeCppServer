#pragma once
#include <functional>
#include <string>

class EventLoop;
class Socket;
class Channel;
class Buffer;

class Connection{
private:
    EventLoop* loop;
    Socket* sock;
    Channel* channel;
    std::function<void(Socket*)> deleteConnectionCallback;
    std::string* inbuffer;
    Buffer* readBuffer;

public:
    Connection(EventLoop* _loop, Socket* _sock);
    ~Connection();

    void echo(int socket);
    void setDeleteConnectionCallback(std::function<void(Socket*)>);
};