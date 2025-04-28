#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "Socket.h"
#include "Channel.h"
#include "Connection.h"
#include "Buffer.h"
#include "util.h"

Connection::Connection(EventLoop* _loop, Socket* _sock) : 
    loop(_loop), sock(_sock), channel(nullptr), inbuffer(new std::string), readBuffer(nullptr){
        channel = new Channel(loop, sock->getFd());
        std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
        channel->setReadCallback(cb);
        channel->setUseThreadPool(true);
        readBuffer = new Buffer();
}

Connection::~Connection(){
    delete channel;
    delete sock;
    delete readBuffer;
}

void Connection::echo(int sockfd){
    char buf[1024];         // 这个 buf 大小可以根据 TCP 缓冲区大小来设置
    while(true){            // 由于使用非阻塞 I/O，读取客户端 buffer，一次读取 buf 大小的数据，直到全部读取完毕
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if(bytes_read > 0){
            readBuffer->append(buf, bytes_read);
        }else if(bytes_read == -1 && errno == EINTR){
            // 客户端正常中断、继续读取
            printf("continue reading");
            continue;
        }else if(bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
            // 非阻塞 I/O，这个条件表示数据全部读取完毕
            printf("finish reading once\n");
            printf("message from client fd %d: %s\n", sockfd, readBuffer->c_str());
            send(sockfd);
            readBuffer->clear();
            break;
        }else if(bytes_read == 0){
            // EOF，客户端断开连接
            printf("EOF, clinet fd %d disconnected\n", sockfd);
            // close(sockfd);          // 关闭 socket 会自动将文件描述符从 epoll 树上移除
            deleteConnectionCallback(sockfd);
            break;
        }else {
            printf("Connection reset by peer\n");
            deleteConnectionCallback(sockfd);       // 会有 bug，注释后单线程无 bug
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(int)> _cb){
    deleteConnectionCallback = _cb;
}

void Connection::send(int sockfd){
    char buf[readBuffer->size()];
    strcpy(buf, readBuffer->c_str());
    int data_size = readBuffer->size();
    int data_left = data_size;
    while(data_left > 0){
        ssize_t bytes_write = write(sockfd, buf + data_size - data_left, data_left);
        if(bytes_write == -1 && errno == EAGAIN){
            break;
        }
        data_left -= bytes_write;
    }
}