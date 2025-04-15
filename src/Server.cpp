#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <functional>
#include <unistd.h>

#include "Server.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"

#define READ_BUFFER 1024

Server::Server(EventLoop* _loop) : loop(_loop){
    Socket* server_sock = new Socket();
    InetAddress* server_addr = new InetAddress("127.0.0.1", 8888);
    server_sock->bind(server_addr);
    server_sock->listen();
    server_sock->setnonblocking();

    Channel* serverChanenl = new Channel(loop, server_sock->getFd());
    // std::bind 用于创建一个可调用对象（函数对象）。它允许将一个函数（或成员函数）和它的参数绑定在一起，生成一个新的可调用对象，这个对象可以在稍后被调用时执行原始函数。
    // 下述语句的作用是：创建一个可调用对象，这个对象绑定了 Server 类的成员函数 newConnection，并将其第一个参数 server_sock 绑定到当前的 Socket 对象
    // 当这个可调用对象被调用时，它会调用 Server 类的成员函数 newConnection，并将 server_sock 作为参数传递给它
    std::function<void()> cb = std::bind(&Server::newConnection, this, server_sock);
    serverChanenl->setCallback(cb);
    serverChanenl->enableReading();
}

Server::~Server(){

}

void Server::handleReadEvent(int sockfd){
    char buf[READ_BUFFER];
    while(true){            // 由于使用非阻塞 I/O，读取客户端 buffer，一次读取 buf 大小的数据，直到全部读取完毕
        // &buf 是一个指向整个数组的指针（char (*)[1024]）
        // bzero函数原型：void bzero(void* s, size_t n)
        // bzero(&buf, sizeof(buf)); 虽然在实际运行时也能正确工作，但类型上不完全匹配 void*，
        // 可能会在某些严格的编译器或编译选项下引发警告，如 -Wall，-pedantic 等编译选项
        // bzero(buf, sizeof(buf)); 也能正常工作，而且更标准、更清晰且更符合类型安全。此时 buf 退化为数组的首地址
        // 另外，bzero 是 C 函数，不是 C++ 的标准函数，在 C++ 中更加推荐使用 std::memset()
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if(bytes_read > 0){
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        }else if(bytes_read == -1 && errno == EINTR){
            // 客户端正常中断、继续读取
            printf("continue reading");
            continue;
        }else if(bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
            // 非阻塞 I/O，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        }else if(bytes_read == 0){
            // EOF，客户端断开连接
            printf("EOF, clinet fd %d disconnected\n", sockfd);
            close(sockfd);          // 关闭 socket 会自动将文件描述符从 epoll 树上移除
            break;
        }
    }
}

void Server::newConnection(Socket* server_sock){
    InetAddress* client_addr = new InetAddress();                       // 会发生内存泄露！没有 delete
    Socket* client_sock = new Socket(server_sock->accept(client_addr)); // 会发生内存泄露！没有 delete
    printf("new client fd %d! IP: %s Port: %d\n", 
        client_sock->getFd(), 
        inet_ntoa(client_addr->addr.sin_addr), 
        ntohs(client_addr->addr.sin_port));
    client_sock->setnonblocking();
    Channel* clientChannel = new Channel(loop, client_sock->getFd());
    std::function<void()> cb = std::bind(&Server::handleReadEvent, this, client_sock->getFd());
    clientChannel->setCallback(cb);
    clientChannel->enableReading();
}