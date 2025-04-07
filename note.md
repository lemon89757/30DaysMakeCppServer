# 30天基于 Cpp 编写服务笔记

> [30dayMakeCppServer](https://github.com/yuesong-feng/30dayMakeCppServer)

## Day 1
1. 计算机网络
   * [ ] 《计算机网络》谢希仁
2. C++
   * [ ] 《Effective C++》侯捷
3. 操作系统
   * [ ] 《现代操作系统》
4. `Unix` 哲学 `KISS`:
   * keep it simple, stupid

## Day 2
1. Linux:
   * [ ] 《Linux 内核设计与实现》Robert Love

## Day 3
1. **多路复用 I/O**：
   - 是一种允许单个线程或进程同时监视多个文件描述符的技术，用于确定哪些 fd 已经准备好进行读写操作。这种技术在处理多个并发 I/O 操作时非常有用，尤其是在编写高性能网络服务器或处理大量并发连接时。I/O 复用和多线程有相似之处，但绝不是一个概念。**I/O 复用是针对 I/O 接口，而多线程是针对 CPU**。
   - 在 Linux 中，`select, poll, epoll` 是三种主要的多路复用 I/O 机制，其中 `select` 和 `poll` 的实现方式类似于**轮询（Polling）方式**，这种方式不需要操作系统提供事件通知机制；而 `epoll` 使用**事件通知机制**来避免轮询，它通过内核提供的事件队列来通知通知程序哪些 fd 已经准备好进行 I/O 操作，从而避免不必要的检查。因此，`epoll` 更适用于处理大量并发连接，性能更高。
2. `epoll` 的使用
   ```C++
   // 1. 创建 epoll 文件描述符
   /* 方式 1
    * 参数表示监听事件的数量，如果超过内核会自动调整，已经被舍弃。
    * 无实际意义，传入一个大于 0 的数即可
    */
   // int epfd = epoll_create(1024);

   /* 方式 2 
    * 参数是一个 flag，一般设为 0
    */   
   int epfd = epoll_create1(0);

   // 2. 设置 epoll 监听的文件描述符
   // epoll 监听事件的描述符会放在一颗红黑树上，将要监听的 I/O 口放入 epoll 红黑树中。就可以监听该 I/O 上的事件
   /* 其中：
    * sockfd 表示需要监听的 I/O 文件描述符
    * ev 是一个 epoll_event 结构体，其中的成员 events 表示需要监听的事件，data 为用户数据
    */
   epoll_clt(epfd, EPOLL_CTL_ADD, sockfd, &ev);    // 添加事件到 epoll
   epoll_clt(epfd, EPOLL_CTL_MOD, sockfd, &ev);    // 修改 epoll 红黑树行的事件
   epoll_clt(epfd, EPOLL_CTL_DEL, sockfd, NULL);   // 删除事件

   // 3. 获取有事件发生的 fd
   // epoll 默认采用 LT 触发模式，即水平触发，只要 fd 上有事件，就会一直通知内核。
   // 这样可以保证所有事件都得到处理，不容易丢失，但可能发生的大量重复通知也会影响 epoll 的性能；
   // ET 模式，即边缘触发，fd 从无事件到有事件的变化会通知内核一次，之后就不会再次通知内核。
   // 这种方式十分高效，可以大大提高支持的并发度，但程序逻辑必须一次性很好地处理该 fd 上的事件，编程比 LT 更繁琐。
   // 注意，ET 模式必须搭配非阻塞式 socket 使用
   /* 其中 
    * events   是一个 epoll_event 结构体数组
    * maxevent 是可供返回的最大事件数量，一般是 events 的大小
    * timeout  表示最大等待时间，设置为 -1 表示一直等待
    */
   int nfds = epoll_wait(epfd, events, maxevents, timeout);
   ```
3. 将服务器改写为 `epoll` 版本的基本思想：
   - 创建 socket fd，并经过 bind、listen 操作；
   - 创建 epoll fd；
   - 设置 epoll 监听 socket fd 的相关事件；例如，监听 fd 上的可读事件，当 fd 上发生可读事件，表示有一个新的客户端连接；
   - 处理对应的事件；例如，accept 客户段连接后，将客户端的 socket fd 添加到 epoll 中，从而监听客户端 socket fd 是否有事件发生，如果发生则处理事件。 
4. Linux：
   * [ ] 《UNIX 网络编程：卷 1》

## Day 4
1. 计算机系统、C/C++ 程序编译、链接
   * [ ] 《深入理解计算机系统（第三版）》——第七章
