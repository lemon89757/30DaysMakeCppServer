#include <unistd.h>
#include <functional>

#include "Server.h"
#include "Socket.h"
#include "Acceptor.h"
#include "Connection.h"

#define READ_BUFFER 1024

Server::Server(EventLoop* _loop) : loop(_loop){
    acceptor = new Acceptor(loop);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server(){
    delete acceptor;
}

void Server::handleReadEvent(int sockfd){
    
}

void Server::newConnection(Socket* sock){
    if(sock->getFd() != -1){
        Connection* conn = new Connection(loop, sock);
        std::function<void(int)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
        conn->setDeleteConnectionCallback(cb);
        connections[sock->getFd()] = conn;
    }
}

void Server::deleteConnection(int sockfd){
    if(sockfd != -1){
        auto it = connections.find(sockfd);
        if(it != connections.end()){
            Connection *conn = it->second;
            connections.erase(sockfd);
            close(sockfd);              // 正常
            // delete conn;             // 会 Segment fault
        }
    }
}