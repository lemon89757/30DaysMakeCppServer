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

## Day 5
* 通过 `Channel` 类来在事件发生时记录详细信息，以便后序作针对处理。`Channel` 类记录了被监听的文件描述符 `fd`、监听 `fd` 的 `Epoll` 实例 `ep`、希望对 `fd` 监听的事件以及 `fd` 实际发生的事件。因此通过 `ep` 的 `poll()` 方法获取到的事件，需要通过 `Channel` 类来实例化以记录详细信息再返回；在通过 `ep` 的 `updateChannel()` 方法进行监听时，也需要将 `Channel` 实例自身绑定在结构体 `struct epoll_event` 的 `data` 属性的 `ptr` 字段上，从而可以在 `poll()` 时重新获取。

## Day 6
1. Reactor 模式（Dispatch 模式）：I/O 多路复用监听事件，收到事件后，根据事件类型发你配（Dispatch）给某个线程/进程。Reactor 模式主要由 Reactor 和处理资源池这两部分组成。相当于在 `Day 5` 获取事件发生的详细信息的情况下，对后续针对性的操作进一步进行分工：例如 Reactor 负责监听和分发事件，包括连接事件、读写事件；处理资源池（Handler）负责处理事件，如 read -> 业务逻辑 -> send；还可以通过 Acceptor 建立连接。
2. Proactor：Reactor 是非阻塞同步网络模式（感知的是可读写事件。相当于是来了事件，操作系统通知应用程序，让应用程序来处理），而 Proactor 是异步网络模式（感知的是已完成的读写事件。相当于是来了事件，操作系统来处理，处理完再通知应用程序）。
3. 阻塞、非阻塞、异步 I/O
   * 阻塞 I/O：需要等待 **内核数据准备好** 和 **数据从内核态拷贝到用户态** 这两个过程（同步过程）；
   * 非阻塞 I/O：在内核数据未准备好时直接返回。当内核数据准备好后，还需等待 **数据从内核态拷贝到用户态** 这一过程（同步过程）；
   * 异步 I/O：上述两个过程都不需要等待；
>[小林哥知乎](https://www.zhihu.com/question/26943938/answer/1856426252)
4. Reactor, Proactor 参考书籍
   * [ ] 《Linux 多线程服务器编程》陈硕——第六章
   * [ ] 《Linux 高性能服务器编程》游双——第八章
5. 事件驱动库
   * [ ] libevent C语言事件驱动库

## Day 7
1. 之前以根据 `Reactor` 模式进行了重构，但当前 `Server` 包含所有的服务器逻辑。为实现 `Server` 作为一个服务器类更加通用、更抽象，对其进行进一步的模块化：
   * 增加一个 `Acceptor` 类，用于分离接受连接这一模块（因为不管对于什么服务，在基于 TCP 通信时，都需要进行连接）；
   * 特点：
     1. 类存在于事件驱动 `EventLoop` 类中，也就是 `Reactor` 模式的 `main-Reactor`（相当于原本处于 `Server` 中的事件驱动循环 `EventLoop` 放在了 `Acceptor` 类中，因为它需要处理连接请求，需要完成对服务器 `socket fd` 的监听）；
     2. **类中的 `socket fd` 就是服务器监听的 `socket fd`，每一个 `Acceptor` 对应一个 `socket fd`**（服务器需要监听多个文件描述符，而特定的文件描述符通过接收外部连接来提供对应的服务）；
     3. 这个类也通过一个独有的 `Channel` 负责分发到 `epoll`，该 `Channel` 的事件处理函数 `handleEvent()` 会调用 `Acceptor` 中的接受连接函数来新建一个 `TCP` 连接。
   * 新的 `TCP` 连接应该由 `Server` 类来创建并管理生命周期，而不是 `Acceptor`（此时 `Acceptor` 更多的是充当消息传递的角色）；同时将处理连接的逻辑代码放在 `Server` 类里也并没有打破服务器的通用性，因为对于所有服务器，都要使用 `Acceptor` 来建立连接。为了实现这一设计，可以用两种方式：1. 使用传统的虚类、虚函数来设计一个接口； 2. C++ 11 的特性：`std::function, std::bind, 右值引用, std::move` 等实现**函数回调**。（*虚函数使用起来比较繁琐，程序的可读性也不够明朗，而 `std::function, std::bind` 等新标准的出现可以完全替代虚函数*）
   * 至此，`Server` 类主要是 实际用于处理连接的业务逻辑和处理其他事件的业务逻辑，而其他跟连接相关的内容交由 `Acceptor` 来处理，`Server` 具有 `Acceptor` 类型对象。
2. C++
* [ ] 《现代 C++ 教程》欧长坤

## Day 8
1. 通过 `Connection` 类来处理、记录和管理通过 `TCP` 连接的客户端。它与 `Acceptor` 类是平行关系，都直接由 `Server` 管理，由一个 `Channel` 分发到 epoll, 通过回调函数处理相应事件。

## Day 9
1. 增加读写缓存区。前面是以固定大小的方式对 TCP 缓冲区进行读取，然后发送给客户端。它在逻辑上有很多不合适的地方：比如，事先并不知道客户端信息的正真大小是多少，只能以固定字节数量（如 1024）读缓冲区去读 TCP 缓冲区；也不能一次性读取所有客户端数据后再统一发给客户端。
2. 为每一个 `Connection` 类分配一个读缓冲区和写缓冲区，从客户端读取来的数据都存放在读缓冲区里。
3. 在对 TCP 缓冲进行读取时，依然有一个 `char buf[]` 缓冲区，用于系统调用 `read` 的读取。该缓冲区太小或太大都可能对性能造成影响（太小读取次数增多，太大资源浪费、单次读取速度慢，**设置为 1 到设备 TCP 缓冲区的大小都可以**）。

## Day 10
1. 关于前面在客户端代码中提到的问题：
   ```C++
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
   ```
   在缓冲区大小固定的情况下，客户端（接收端）的缓冲区大小需要大于等于服务器端（发送端）缓冲区大小，主要有以下几个原因：
   1. 防止长数据被截断。当客户端缓冲区小导致只能接收到部分发送的数据时，即使剩余的数据存在于套接字通信的缓冲区里，但是客户端可能意识不到该部分数据的存在。此时需要通过对应的协议保证数据的完整，且客户端要能够正确处理缓冲区长度不足时的问题（可以通过多次读取、拼接的方式）；
   2. 系统调用开销。如果需要对一个长数据读取多次来实现通信的话，会导致多次系统调用 `read`，从而降低通信的效率；
   3. 数据处理复杂性。另一方面，关于长数据需要多次读取来拼凑成一个完整的数据增加了客户端数据处理的复杂性。客户端需要维护一个逻辑缓冲区，用于存储从多次 `read` 调用中逐步读取的数据，直到完整地拼接出完整的消息（需要有协议来提供如果判断消息是否完整）。
   因此，在缓冲区大小固定的情况下，最佳实践还是：客户端的缓冲区大小最好与服务器端的缓冲区大小一致，或者至少足够大以容纳单次发送的最大数据量。
2. 提供线程池服务，解决前面服务器端单线程（`EventLoop` 线程）处理连接、客户端业务请求的问题，提高服务器端的效率。
3. 线程池中线程数量一般是 `CPU` 核数（物理支持的最大并发数）；
4. 线程池实现需要注意的点：1. **在多线程环境下，任务队列的读写操作都应该考虑互斥锁**；2. 当任务队列为空时 CPU 不应该不断轮询，这样耗费了 CPU 资源；3. 在线程池析构时，需要注意将已经添加的所有任务执行完，最好不要采用外部的暴力 `kill`，而是**让每个线程从内部自动退出**。与第一点对应，使用 `std::mutex` 来对任务队列进行加锁和解锁；与第二点对应，使用了条件变量 `std::condition_variable`。

## Day 11
1. 优化线程池。通过使用**右值移动、完美转发**等阻止对任务队列进行添加、取出操作时存在的拷贝，提高线程池性能。通过**模板编程**解决线程池只能接受 `std::function<void()>` 类型的参数（需要事先使用 `std::bind()` ，并且无法得到返回值）。
2. 模板内容不能放在 `cpp` 文件中，因为 `C++` 编译器不支持模板的分离编译。
3. 关于模板和 `C++ 11` 相关内容见《现代 C++ 教程》及其他书籍或教程，具体的几个点：
   * [ ] 通过 `->` 明确函数的返回值类型；
   * [ ] `template<class F, class... Args>` 中 `class` 与 `typename` 的区别，`typename` 的用法；
   * [ ] 参数包 `...Args`（变参数）的用法；
   * [ ] 右值引用与完美转发，`std::forward`；
   * [ ] `std::future` 对象；
   * [ ] `std::result_of`, `std::result_of<>::type` 及 `std::result_of<F(Args...)>::type()`（表明它是一个可以被调用的对象）；
   * [ ] `std::make_shared`, `std::packaged_task` 概念及用法；
   