
//============================================================================
// Name        : threadControl.hpp
// Author      : Fengcheng Yu
// Date        : 2024.4.10
// Description : 本项目中client和server都需要维护的tc对象设计
//============================================================================

#ifndef __YUFC_THREAD_CONTROL__
#define __YUFC_THREAD_CONTROL__

#include "./comm.hpp"
#include "./lockGuard.hpp"
#include "./thread.hpp"
#include <queue>
#include <type_traits>
#include <vector>

// 线程池本质就是一个生产者消费者模型
template <class T>
class thread_control {
private:
    std::vector<thread*> __worker_threads; // worker线程
    int __thread_num; // worker线程数量
    std::queue<T> __cache; // 任务队列->也就是20个slot的缓冲区
    size_t __cache_max_size = CACHE_MAX_SIZE_DEFAULT; // 缓冲区最大的大小
    pthread_mutex_t __cache_lock; // 多个线程访问缓冲区的锁
    pthread_cond_t __cache_empty_cond; // 判断缓冲区是否为空
    pthread_cond_t __cache_full_cond; // 判断缓冲区是否满了
    thread* __connector_thread; // connector线程，只有一个
    int __fd; // 管道文件的fd
    double __lambda; // 负指数参数，消费频率/生产频率
public:
    bool __quit_signal = false; // 控制进程结束

public:
    // 这里可以继续封装成对象
    thread_control(void* (*worker)(void*) = nullptr, void* (*connector)(void*) = nullptr, int thread_num = THREAD_NUM_DEFAULT, int fd = -1, double lambda = 1.0)
        : __thread_num(thread_num)
        , __fd(fd)
        , __lambda(lambda) {
        static_assert(std::is_same<T, std::string>::value, "Class template parameter T must be std::string"); // 只能允许字符串
        if (worker == nullptr || connector == nullptr) {
            logMessage(FATAL, "worker function is nullptr or connector fucntion is nullptr");
            exit(1);
        }
        // 初始化同步变量和锁
        pthread_mutex_init(&__cache_lock, nullptr);
        pthread_cond_init(&__cache_empty_cond, nullptr);
        pthread_cond_init(&__cache_full_cond, nullptr);
        for (int i = 1; i <= __thread_num; i++) // 三个线程去进行worker任务
        {
            __worker_threads.push_back(new thread(i, worker /*线程要去干的事情*/, this /*这里可以传递this，作为thread的args*/));
        }
        // 一个线程去执行connector任务
        __connector_thread = new thread(0, connector, this);
    }
    ~thread_control() {
        // 等待所有worker线程
        for (auto& iter : __worker_threads) {
            iter->join(); // 这个线程把自已join()一下
            delete iter;
        }
        // 等待connector线程
        __connector_thread->join();
        // 销毁同步变量和锁
        pthread_mutex_destroy(&__cache_lock);
        pthread_cond_destroy(&__cache_empty_cond);
        pthread_cond_destroy(&__cache_full_cond);
    }
    void run() {
        __connector_thread->start();
        logMessage(NORMAL, "%s %s", __connector_thread->name().c_str(), "start\n");
        for (auto& iter : __worker_threads) {
            iter->start();
            logMessage(NORMAL, "%s %s", iter->name().c_str(), "start\n");
        }
        // 现在所有的线程都启动好了，要控制退出的逻辑
        while (1)
            // 现在各个线程都在运行
            if (__quit_signal) // 监听退出信号
                break;
    }

public:
    /*
        需要一批，外部成员访问内部属性的接口提供给static的routine，不然routine里面没法加锁
        下面这些接口，都是没有加锁的，因为我们认为，这些函数被调用的时候，都是在安全的上下文中被调用的
        因为这些函数调用之前，已经加锁了，调用完，lockGuard自动解锁
    */
    pthread_mutex_t* get_mutex() { return &__cache_lock; } // 获取互斥锁
    pthread_cond_t* get_empty_cond() { return &__cache_empty_cond; } // 获取缓存空的同步变量
    pthread_cond_t* get_full_cond() { return &__cache_full_cond; } // 获取缓存满的同步变量
    std::queue<T>& get_queue() { return this->__cache; } // 获取任务队列（缓存）
    void wait_empty_cond() { pthread_cond_wait(&__cache_empty_cond, &__cache_lock); } // 等待缓存为空的同步变量
    void wait_full_cond() { pthread_cond_wait(&__cache_full_cond, &__cache_lock); } // 等待缓存为满的同步变量
    bool is_empty() { return __cache.empty(); } // 判断缓存是否为空
    bool is_full() { return __cache.size() >= __cache_max_size; } // 判断缓存是否为满
    int get_fd() { return this->__fd; } // 获取该tc对象的文件描述符
    double get_lambda() { return this->__lambda; } // 获取该tc对象的lambda参数
    T get_task() {
        T t = __cache.front();
        __cache.pop();
        return t; // 拷贝返回
    }
};

#endif