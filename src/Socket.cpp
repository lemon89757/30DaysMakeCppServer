#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "util.h"
#include "Socket.h"
#include "InetAddress.h"

Socket::Socket() : fd(-1){
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(-1 == fd, "socket create error");
}

Socket::Socket(int _fd) : fd(_fd){
    errif(-1 == fd, "socket create error");
}

Socket::~Socket(){
    if(-1 != fd){
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress* _addr){
    int err;
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddr_len();
    // ::bind 是 C++ 中的全局作用域运算符，用于明确调用全局的 bind 函数
    // 而不是类中的其他同名函数
    err = ::bind(fd, (sockaddr*)&addr, addr_len);
    errif(-1 == err, "socket bind error");
    _addr->setInetAddr(addr, addr_len);
}

void Socket::listen(){
    int err;
    err = ::listen(fd, SOMAXCONN);
    errif(-1 == err, "socket listen error");
}

void Socket::setnonblocking(){
    int status = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, status | O_NONBLOCK);
}

int Socket::accept(InetAddress* _addr){
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    bzero(&addr, sizeof(addr));
    int client_fd = ::accept(fd, (sockaddr*)&addr, &addr_len);
    errif(-1 == client_fd, "socket accept error");
    _addr->setInetAddr(addr, addr_len);
    return client_fd;
}

void Socket::connect(InetAddress* _addr){
    int ret;
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddr_len();
    ret = ::connect(fd, (sockaddr*)&addr, addr_len);
    errif(-1 == ret, "socket connect error");
}

int Socket::getFd(){
    return fd;
}