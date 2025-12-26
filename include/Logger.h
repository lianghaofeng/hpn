#pragma once

#include<cstdio>
#include<cstdarg>

enum LogLevel {TRACE, DEBUG, INFO, WARN, ERROR, FATAL};

class Logger{
public:
    static void setLogLevel(LogLevel level);
    static void log(LogLevel level, const char* fmt, ...);
};

#define LOG_TRACE(...) Logger::log(TRACE, __VA_ARGS__)
#define LOG_INFO(...) Logger::log(INFO, __VA_ARGS__)
#define LOG_ERROR(...) Logger::log(ERROR, __VA_ARGS__)
