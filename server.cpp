#include <stdio.h>
// #include <string.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>

#include "util.h"
#include "Epoll.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void handleReadEvent(int);
void setnonblocking(int fd){
    // 在这里没有错误处理，todo
    int status = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, status | O_NONBLOCK);
}

int main(){
    Socket* server_sock = new Socket();
    InetAddress* server_addr = new InetAddress("127.0.0.1", 8888);
    server_sock->bind(server_addr);
    server_sock->listen();
    
    Epoll* ep = new Epoll();
    server_sock->setnonblocking();
    Channel *serverChannel = new Channel(ep, server_sock->getFd());
    serverChannel->enableReading();

    while(true){
        std::vector<Channel*> activeChannels = ep->poll();
        int nfds = activeChannels.size();
        for(int i = 0; i < nfds; i++){
            int chfd = activeChannels[i]->getFd();
            if(chfd == server_sock->getFd()){
                // 新客户端连接
                InetAddress* client_addr = new InetAddress();                       // 会发生内存泄漏！没有delete.
                Socket *client_sock = new Socket(server_sock->accept(client_addr)); // 会发生内存泄漏！没有delete.
                printf("new client fd %d! IP: %s Port: %d\n", 
                    client_sock->getFd(), inet_ntoa(client_addr->addr.sin_addr), ntohs(client_addr->addr.sin_port));
                client_sock->setnonblocking();
                Channel* clientChannel = new Channel(ep, client_sock->getFd());
                clientChannel->enableReading();
            }else if(activeChannels[i]->getRevents() & EPOLLIN){
                // 可读事件
                handleReadEvent(activeChannels[i]->getFd());
            }else{
                // 其他事件
                printf("something else happened\n");
            }
        }
    }

    delete server_sock;
    delete server_addr;
    // 没有 delete ep ?

    return 0;
}

void handleReadEvent(int sockfd){
    char buf[READ_BUFFER];
    while(true){
        // 由于使用非阻塞 I/O，读取客户端 buffer，一次读取 buf 大小数据，直到全部读取完毕
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if(bytes_read > 0){
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        }else if(bytes_read == -1 && errno == EINTR){
            // 客户端正常中断，继续读取
            printf("continue reading");
            continue;
        }else if(bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
            // 非阻塞 I/O，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        }else if(bytes_read == 0){
            // EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd);      // 关闭 socket 会自动将文件描述符从 epoll 树上移除
            break;
        }
    }
}
