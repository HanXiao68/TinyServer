# TinyServer

## Introduction  

本项目为C++11编写的Web服务器，使用了智能指针，支持解析get、post请求，可处理静态资源，支持HTTP长连接，并实现了日志模块，记录服务器运行状态。 
之后，实现了多传感器的多线程并行机制，使用互斥锁等机制实现线程间的同步。

#  项目目的

### 最初的设想
这个项目旨在为了笔者在机器人所的多模态感知融合系统，设计开发一个后端服务器，以此来支持激光雷达，相机，深度相机，IMU等传感器的数据接受处理和特征传输的实时性，可靠性要求。因此，来设计开发这个高性能，多线程的后台服务器。该项目是将无人驾驶或者说无人配送领域中的核心部分抽取出来，使用水滴移动机器人作为移动平台代替无人汽车，以最小的代价来攻克难题，研究多模态的数据处理问题和后续的算法选型，实现。

### 做了什么
> * 实现了初级版的echo反射服务器，服务器端能够打印显示client发送来的数据
> * 实现解析http协议，用http1.1 实现断点续传，优化传输视频等大文件。可以作为webServer来使用。
>  * 加入定时器，满足特定功能。比如：在30s内没有接收到模态的数据或者特征，就释放数据库连接到连接池中。

### 流程是什么
<img src="https://github.com/HanXiao68/TinyServer/blob/master/PROJECT_FlowChart.png" width="775"/>

### 达到什么指标
> * 能够满足实时性要求，接收传感器数据，和完成传感器时间戳对齐，以及数据融合处理。
> * 作为webserver，经过webbench的压测，能够在QPS，TPS等指标上达到 XXX级别。
> * 能够通过这个服务，运用到后台开发的相关知识，锻炼自己的开发能力。

### 不足和展望
> * 由于笔者目前的条件有限，只能在单机上做多线程。其实这个问题，在例如很多的大厂肯定有着完备的分布式 服务器来支持无人驾驶。因此，下一个版本计划，将服务器做成分布式，能够实现负载均衡，加入Nginx。
> * 实现视频流和路况避障的实时传输和快速处理，并且保证高可靠性。

测试页：

| Part Ⅰ | Part Ⅱ | Part Ⅲ | Part Ⅳ | Part Ⅴ | Part Ⅵ |
| :--------: | :---------: | :---------: | :---------: | :---------: | :---------: |
| [并发模型](https://github.com/HanXiao68/TinyServer/blob/master/%E5%B9%B6%E5%8F%91%E6%A8%A1%E5%9E%8B.md)|[连接的维护](https://github.com/HanXiao68/TinyServer/blob/master/%E8%BF%9E%E6%8E%A5%E5%A4%84%E7%90%86.md)| [测试及改进](https://github.com/HanXiao68/TinyServer/blob/master/%E6%B5%8B%E8%AF%95.md) | [项目目的](https://github.com/linyacool/WebServer/blob/master/%E9%A1%B9%E7%9B%AE%E7%9B%AE%E7%9A%84.md) | [项目的思考](https://github.com/HanXiao68/TinyServer/blob/master/%E9%A1%B9%E7%9B%AE%E7%9A%84%E6%80%9D%E8%80%83)

## Technical points
* 使用Epoll边沿触发的IO多路复用技术，非阻塞IO，使用Reactor模式
* 使用多线程充分利用多核CPU，并使用线程池避免线程频繁创建销毁的开销
* 使用基于小根堆的定时器关闭超时请求
* 主线程只负责accept请求，并以Round Robin的方式分发给其它IO线程(兼计算线程)，锁的争用只会出现在主线程和某一特定线程中
* 使用eventfd实现了线程的异步唤醒
* 为减少内存泄漏的可能，使用智能指针等RAII机制
* 使用状态机解析了HTTP请求
* 支持优雅关闭连接

---
## 事件处理模式

### Reactor模式

<img src="https://github.com/HanXiao68/libevent/blob/master/image/reactor.png" width="775"/>
    要求主线程（IO处理单元）只负责监听文件描述符fd上是否有时间发生，有的话立即将事件通知工作线程（逻辑单元）。除此之外，主线程不做任何其他实质性的工作。
    读写数据，接受新的连接，以及处理客户请求均在工作线程中完成。

    使用同步非阻塞IO多路复用模型（以epoll为例）实现的Reactor模式的工作流程是：
        1.主线程往epoll内核事件表中注册socket上的读就绪事件。
        2.主线程调用epoll_wait等待socket上有数据可读。
        3.当socket上有数据可读时，epoll_wait通知主线程。主线程将socket可读事件放入请求队列。
        4.睡眠在请求队列上的某个工作线程被唤醒，他从socket读取数据，并处理客户请求，然后往epoll内核事件表中注册该socket上的写就绪事件。
        5.主线程调用epoll_wait等待socket可写。
        6.当socket可写时，epoll_wait通知主线程。主线程将socket可写事件放入请求队列。
        7.睡眠在请求队列撒花姑娘的某个工作线程被唤醒，它往socket上写入服务器处理客户请求的结果。
        8.当socket上有数据可写时，epoll_wait通知主线程。主线程将socket可写事件放入请求队列。
---

### proactor模式
    主线程和内核 读写数据，接受连接。    工作线程只处理业务逻辑，客户端请求。
---

压力测试
-------------
在关闭日志后，使用Webbench对服务器进行压力测试，对listenfd和connfd采用ET和LT模式，均可实现上万的并发连接，下面列出的是两者组合后的测试结果. 

> * 进程监控。占用86.7%  CPU。

<div align=center><img src="https://github.com/HanXiao68/TinyServer/blob/master/test_pressure_result/top.png" height="201"/> </div>

> * Reactor，LT + ET，4669 QPS

<div align=center><img src="https://github.com/HanXiao68/TinyServer/blob/master/test_pressure_result/500.png" height="201"/> </div>


> * 并发连接总数：93357
> * 访问服务器时间：20s
> * 所有访问均成功

**注意：** 使用本项目的webbench进行压测时，若报错显示webbench命令找不到，将可执行文件webbench删除后，重新编译即可。


