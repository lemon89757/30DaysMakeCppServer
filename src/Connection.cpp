#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "Socket.h"
#include "Channel.h"
#include "Connection.h"

#define READ_BUFFER 1024

Connection::Connection(EventLoop* _loop, Socket* _sock) : 
    loop(_loop), sock(_sock), channel(nullptr){
        channel = new Channel(loop, sock->getFd());
        std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
        channel->setCallback(cb);
        channel->enableReading();
}

Connection::~Connection(){
    delete channel;
    delete sock;
}

void Connection::echo(int sockfd){
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

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _cb){
    deleteConnectionCallback = _cb;
}