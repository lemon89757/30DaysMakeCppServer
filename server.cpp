#include <stdio.h>
// #include <string.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "util.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd){
    // 在这里没有错误处理，todo
    int status = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, status | O_NONBLOCK);
}

int main(){
    int ret;
    // protocol = 0: 表示根据前面的两个参数自动推导协议类型。
    // 设置为 IPPROTO_TCP 和 TPPROTO_UDP，分别表示 TCP 和 UDP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in server_addr;
    // 使用 bzero 初始化结构体
    // 《effective C++》准则：
    // 条款 4：确定对象被使用前已先被初始化。如果不清空，使用 gdb 调试器查看 addr 内的变量，会是一些随机值，未来可能导致意想不到的问题。
    // 条款 1：视 C++ 为一个语言联邦。把 C 和 C++ 看作两种语言，写代码时需要清楚地知道自己在写 C 还是 C++。如果在写 C，请包含头文件 <string.h>。如果在写 C++，请包含 <cstring>。
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8888);

    ret = bind(sockfd, (sockaddr*)&server_addr, sizeof(server_addr));
    errif(ret == -1, "socket bind error");

    ret = listen(sockfd, SOMAXCONN);
    errif(ret == -1, "socket listen error");

    // open an epoll file description
    int epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");

    struct epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));
    bzero(&ev, sizeof(ev));
    ev.data.fd = sockfd;
    // epoll 默认采用 LT 触发方式，即水平触发。只要 fd 上有事件，就会一直通知内核
    // ET 模式，即边缘触发。fd 从无事件到有事件的变化会通知内核一次，之后就不再通知内核
    // 实际上，接收连接最好不要用 ET 模式
    ev.events = EPOLLIN | EPOLLET;
    setnonblocking(sockfd);
    // this system call is used to add, modify, or remove entries in the 
    // interest list of epoLL() instance refered to by the file descriptor epfd.
    // EPOLL_CTL_ADD: add an entry to the interest list of the epoll file descriptor, epfd.
    // the entry includes the file descriptor, fd, a reference to the corresponding open file description,
    // and settings specified in event.
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    errif(ret == -1, "epoll add an entry error");

    while(true){
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        errif(nfds == - 1, "epoll wait error");
        for(int i = 0; i < nfds; i++){
            if(events[i].data.fd == sockfd){
                /* 新客户端连接 */
                struct sockaddr_in client_addr;
                bzero(&client_addr, sizeof(client_addr));
                socklen_t client_addr_len = sizeof(client_addr);

                int client_sockfd = accept(sockfd, (sockaddr*)&client_addr, &client_addr_len);
                errif(client_sockfd == -1, "socket accept error");
                printf("new client fd %d! IP: %s Port: %d\n", client_sockfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                bzero(&ev, sizeof(ev));
                ev.data.fd = client_sockfd;
                ev.events = EPOLLIN | EPOLLET;
                setnonblocking(client_sockfd);
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockfd, &ev);
                errif(ret == -1, "epoll add an entry error");
            }else if(events[i].events & EPOLLIN){
                /* 可读事件 */
                char buf[READ_BUFFER];
                while(true){
                    // 由于使用非阻塞 I/O，读取客户端 buffer，一次读取 buf 大小数据，直到全部读取完毕
                    bzero(&buf, sizeof(buf));
                    ssize_t read_bytes = read(events[i].data.fd, buf, sizeof(buf));
                    if(read_bytes > 0){
                        printf("message from client fd %d: %s\n", events[i].data.fd, buf);
                        write(events[i].data.fd, buf, sizeof(buf));
                    }else if(read_bytes == -1 && errno == EINTR){
                        // 客户端正常中断，继续读取
                        printf("continue reading");
                        continue;
                    }else if(read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
                        // 非阻塞 I/O，这个条件表示数据全部读取完毕
                        printf("finish reading once, errno: %d\n", errno);
                        break;
                    }else if(read_bytes == 0){
                        // EOF，客户端断开连接
                        printf("EOF, client fd %d disconnected\n", events[i].data.fd);
                        // 关闭 socket 会自动将文件描述符从 epoll 树上移除
                        close(events[i].data.fd);
                        break;
                    }
                }
            }else{
                // 其他事件，之后的版本实现
                printf("something else happened\n");
            }
        }
    }
    
    close(sockfd);
    return 0;
}
