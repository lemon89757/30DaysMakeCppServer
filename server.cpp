#include <stdio.h>
// #include <string.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "util.h"

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

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    bzero(&client_addr, sizeof(client_addr));

    // 此时获取到的客户端 socket fd 本质上与客户端本身的 socket fd 不是同一个
    // 但是它们指向同一个 tcp 连接
    // 它们分别位于不同的进程中，所以不是同一个 fd（每个进程有自己的文件打开表）
    // 因此在服务器端关闭的客户端 socket fd，也不是关闭了客户端的 socket fd
    int client_sockfd = accept(sockfd, (sockaddr*)&client_addr, &client_addr_len);
    errif(client_sockfd == -1, "socket accept error");
    printf("new client fd %d! IP: %s Port %d\n", client_sockfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    while(true){
        char buf[1024];
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(client_sockfd, buf, sizeof(buf));
        if(read_bytes > 0){
            printf("message from client fd %d: %s\n", client_sockfd, buf);
            write(client_sockfd, buf, sizeof(buf));
        }else if(read_bytes == 0){
            printf("client fd %d disconnected\n", client_sockfd);
            close(client_sockfd);
            break;
        }else if(read_bytes == -1){
            close(client_sockfd);
            errif(true, "socket read error");
        }
    }
    
    close(sockfd);
    return 0;
}
