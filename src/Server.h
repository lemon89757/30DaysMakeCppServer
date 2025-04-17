#pragma once

#include <map>

class EventLoop;
class Socket;
class Acceptor;
class Connection;

class Server{
private:
    EventLoop* loop;
    Acceptor* acceptor;
    std::map<int, Connection*> connections; // key-文件描述符，value-对应的 Conenction 实例
public:
    Server(EventLoop*);
    ~Server();

    void handleReadEvent(int);
    void newConnection(Socket* server_sock);
    void deleteConnection(Socket* sock);
};