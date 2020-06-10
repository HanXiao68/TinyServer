#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

class sem
{
public:
    sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0) 初始化一个没有命名的信号量
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_sem); 销毁信号量
    }
    bool wait()
    {
        return sem_wait(&m_sem) == 0; 将信号量减一。信号量为0时，sem_wait阻塞。
    }
    bool post()
    {
        return sem_post(&m_sem) == 0;信号量加一，信号量大于0时。唤醒调用sem_post的线程。
    }

private:
    sem_t m_sem;
};
class locker  互斥锁，也叫互斥量。 lock() 和unlock（） 之间是保护的代码段。确保一个时刻只有一个线程访问。
进入lock()时，加锁。出unlock()时，唤醒等待在这的线程。
{
public:
    locker()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    pthread_mutex_t *get()
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};
class cond 条件变量： 线程之间的通知机制，当某个共享数据达到某个值时，唤醒等待的线程
{
public:
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0) 初始化条件变量
        {
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond); 销毁条件变量
    }
    //条件变量使用需要配合 互斥锁来使用，需要一次加锁，解锁操作。
    bool wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, m_mutex);
        pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
#endif
