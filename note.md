# 30天基于 Cpp 编写服务笔记

> [30dayMakeCppServer](https://github.com/yuesong-feng/30dayMakeCppServer)

## Day 1
1. 计算机网络
   * [ ] 《计算机网络》谢希仁
2. C++
   * [ ] 《Effective C++》侯捷
3. 操作兄台
   * [ ] 《现代操作系统》
4. `Unix` 哲学 `KISS`:
   1. keep it simple, stupid

## Day 2
1. Linux:
   * [ ] 《Linux 内核设计与实现》Robert Love

## Day 3
1. **多路复用 I/O**：是一种允许单个线程或进程同时监视多个文件描述符的技术，用于确定哪些 fd 已经准备好进行读写操作。这种技术在处理多个并发 I/O 操作时非常有用，尤其是在编写高性能网络服务器或处理大量并发连接时。在 Linux 中，`select, poll, epoll` 是三种主要的多路复用 I/O 机制，其中 `select` 和 `poll` 的实现方式类似于轮询（Polling）方式，这种方式不需要操作系统提供事件通知机制；而 `epoll` 使用事件通知机制来避免轮询，它通过内核提供的事件队列来通知通知程序哪些 fd 已经准备好进行 I/O 操作，从而避免不必要的检查。因此，`epoll` 更适用于处理大量并发连接，性能更高。
2. Linux：
   * [ ] 《UNIX 网络编程：卷 1》
