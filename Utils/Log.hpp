
//============================================================================
// Name        : Log.hpp
// Author      : 俞沣城
// Date        : 2024.4.10
// Description : 日志函数的封装
//============================================================================

#ifndef __YUFC_LOG_HPP__
#define __YUFC_LOG_HPP__

// 日志是有日志级别的
// 不同的级别代表不同的响应方式

#define DEBUG 0
#define NORMAL 1
#define WARNING 2
#define ERROR 3
#define FATAL 4

#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <time.h>

// 获取对应日志等级的字符串，方便打印
const char* gLevelMap[] = {
    "DEBUG",
    "NORMAL",
    "WARNING",
    "ERROR",
    "FATAL"
};

/*
    LOGFILE为""时，日志输出到stdout中国呢
    LOGFILE为有效路径时，日志输出到路径指定的文件中
*/
#define LOGFILE ""

void logMessage(int level, const char* format, ...) {
#ifdef __DEBUG_SHOW
    if (level == DEBUG)
        return;
#else
    char stdBuffer[1024];
    time_t timestamp = time(nullptr);
    snprintf(stdBuffer, sizeof stdBuffer, "[%s] [%ld] ", gLevelMap[level], timestamp);
    char logBuffer[1024]; // 自定义部分
    va_list args;
    va_start(args, format); // 初始化一下
    vsnprintf(logBuffer, sizeof logBuffer, format, args); // 可以直接格式化到字符串中来
    va_end(args);
#endif
    FILE* fp = nullptr;
    if (LOGFILE == "")
        fp = stdout;
    else
        fp = fopen(LOGFILE, "w");
    fprintf(fp, "%s%s", stdBuffer, logBuffer);
    fflush(fp); // 刷新一下缓冲区
    if (fp == stdout)
        return;
    fclose(fp); // 如果是stdout就不要close了，close了还得了
}

#endif