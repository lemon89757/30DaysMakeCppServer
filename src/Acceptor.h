#pragma once
#include <functional>

class EventLoop;
class Socket;
class InetAddress;
class Channel;

class Acceptor{
private:
    EventLoop* loop;
    Socket* sock;
    InetAddress* addr;
    Channel* acceptChannel;

public:
    Acceptor(EventLoop* _loop);
    ~Acceptor();
    void acceptConnection();
    void setNewConnectionCallback(std::function<void(Socket*)>);
    std::function<void(Socket*)> newConnectionCallback;
};