

//============================================================================
// Name        : comm.hpp
// Author      : 俞沣城
// Date        : 2024.4.10
// Description : 配置文件头文件
//============================================================================

#ifndef _COMM_HPP_
#define _COMM_HPP_

#include "./Log.hpp"
#include <assert.h>
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <random>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// ---------------------------------------- 管道文件相关配置 ----------------------------------------
#define MODE 0666 // 定义管道文件的权限
#define SIZE 128 // 管道设计的大小要足够大，避免在测试打满server缓冲区时崩溃
std::string ipcPath = "../Utils/fifo.ipc";
// ----------------------------------- threadControl对象相关配置 -----------------------------------
#define THREAD_NUM_DEFAULT 3 // 默认三个线程
#define CACHE_MAX_SIZE_DEFAULT 20 // 默认缓冲区最大为20
// ----------------------------------- 通信相关配置 -----------------------------------
#define MESG_NUMBER 50 // 定义发n条消息之后，Client自动退出
#define WAIT_IO_DONE 2 // 最后client和server会打印程序运行时间，避免前面有io没有结束，导致打印时间的语句不实在最后一行，所以在打印最后的时间语句前统一睡眠，等待io结束
#endif