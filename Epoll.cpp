#include <string.h>
#include <unistd.h>

#include "Epoll.h"
#include "util.h"
#include "Channel.h"

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

std::vector<Channel*> Epoll::poll(int timeout){
    std::vector<Channel*> activeEvents;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(-1 == nfds, "epoll wait error");
    for(int i = 0; i < nfds; i++){
        Channel* ch = (Channel*)events[i].data.ptr;
        ch->setRevents(events[i].events);
        activeEvents.emplace_back(ch);
    }
    return activeEvents;
}

void Epoll::updateChannel(Channel* channel){
    int fd = channel->getFd();
    struct epoll_event ev;
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if(!channel->getInEpoll()){
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->setInEpoll();
    }else{
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
    }
}