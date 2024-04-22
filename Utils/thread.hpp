
//============================================================================
// Name        : thread.hpp
// Author      : 俞沣城
// Date        : 2024.4.10
// Description : 对线程的名字和线程的pthread_t进行封装
//============================================================================

#ifndef __YUFC_THREAD_HPP__
#define __YUFC_THREAD_HPP__

#include "./comm.hpp"

typedef void* (*func_t_)(void*); // 函数指针

class __thread_data {
public:
    void* __args;
    std::string __name;
};

class thread {
private:
    std::string __name; // 线程的名字
    pthread_t __tid; // 线程tid
    func_t_ __func; // 线程要调用的函数
    __thread_data __t_data; // 这个线程对应的数据
public:
    thread(int num, func_t_ callback, void* args)
        : __func(callback) {
        char name_buffer[64] = { 0 };
        snprintf(name_buffer, sizeof name_buffer, "Thead-%d", num);
        __name = name_buffer;
        __t_data.__args = args;
        __t_data.__name = __name;
    }
    ~thread() = default;
    // 启动线程
    void start() { pthread_create(&__tid, nullptr, __func, &__t_data); }
    // 等待线程结束
    void join() { pthread_join(__tid, nullptr); }
    // 返回线程的名字
    std::string name() { return __name; }
};

#endif