#include <string.h>
#include <unistd.h>

#include "Epoll.h"
#include "util.h"

#define MAX_EVENTS 1000

Epoll::Epoll() : epfd(-1), events(nullptr){
    epfd = epoll_create1(0);
    errif(-1 == epfd, "epoll create error");
    events = new epoll_event[MAX_EVENTS];
    bzero(events, sizeof(*events) * MAX_EVENTS);
}

Epoll::~Epoll(){
    if(-1 == epfd){
        close(epfd);
        epfd = -1;
    }
    delete [] events;
}

void Epoll::addFd(int fd, uint32_t op){
    int err;
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = fd;
    ev.events = op;

    err = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    errif(-1 == err, "epoll add event error");
}

std::vector<epoll_event> Epoll::poll(int timeout){
    std::vector<epoll_event> activeEvents;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(-1 == nfds, "epoll wait error");
    for(int i = 0; i < nfds; i++){
        activeEvents.emplace_back(events[i]);
    }
    return activeEvents;
}