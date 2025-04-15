// #include <string.h>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "src/util.h"

#define BUFFER_SIZE 1024

int main(){
    int ret;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);

    // 客户端不进行 bind 操作
    // bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr));

    ret = connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr));
    errif(ret == -1, "socket connect error");

    while(true){
        // 在这个版本中，buf 大小必须大于等于服务器端 buf 大小，不然会出错
        // todo: why?
        char buf[BUFFER_SIZE];
        bzero(&buf, sizeof(buf));
        scanf("%s", buf);
        ssize_t write_bytes = write(sockfd, buf, sizeof(buf));
        if(write_bytes == -1){
            printf("socket already disconnect, can't write any more!\n");
            break;
        }
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if(read_bytes > 0){
            printf("message from server: %s\n", buf);
        }else if(read_bytes == 0){
            printf("server socket disconnected!\n");
            break;
        }else if(read_bytes == -1){
            close(sockfd);
            errif(true, "socket read error");
        }
    }

    close(sockfd);
    return 0;
}

