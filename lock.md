### 1.lock 进程通信建立锁的头文件  。
### 锁机制的功能：实现线程同步机制  确保任一时刻只有一个线程能进入目标代码段
### 封装：将锁的创建和删除封装在构造和析构函数中，实现RAII机制。
locker.h 包括三个类
信号量
    构造器
        sem()    
    析构器
        sem_destroy(& m_set)
    两种操作：
        wait()等待    post()
互斥锁

条件锁
#### RAII
把锁的创建和销毁封装在 类的构造和析构函数中。实现RAII机制------资源获取即初始化。核心思想是：将资源或状态和对象的声明周期绑定，通过c++语言的机制。进入生命周期时，初始化，出了生命周期时，销毁。

### 1.lock 进程通信建立锁的头文件  。实现线程同步机制  
locker.h 包括三个类
### 信号量
> * 两个构造器
        sem()    sem(int num)
> * 一个析构器
        sem_destroy(& m_set)
> * 两种操作：
        wait()等待    post()
  * 信号量的取值是任意自然数，最常用的是0和1.
    sem_init 初始化一个没有命名的信号量
    sem_destroy 销毁信号量
    sem_wait  将信号量减一。信号量为0时，sem_wait阻塞。
    sem_post  信号量加一，信号量大于0时。唤醒调用sem_post的线程。
###  互斥锁
* 互斥锁，也叫互斥量。 lock() 和unlock（） 之间是保护的代码段。确保一个时刻只有一个线程访问。
进入lock()时，加锁。出unlock()时，唤醒等待在这的线程。
> * 操作有：构造器，析构器，（实现RAII） ，

pthread_mutex_lock(&m_mutex) 上锁

pthread_mutex_unlock(&m_mutex) 下锁

get() 获得锁对象







### 条件锁

线程之间的通知机制，当某个共享数据达到某个值时，唤醒等待的线程

pthread_cond_init(&m_cond, NULL) 
初始化条件变量


pthread_cond_destroy(&m_cond); 
销毁条件变量





