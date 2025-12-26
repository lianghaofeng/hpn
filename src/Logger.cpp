#include "Logger.h"
#include <ctime>
#include <cstring>

static LogLevel g_logLevel = INFO;

void Logger::setLogLevel(LogLevel level){
    g_logLevel = level;
}

void Logger::log(LogLevel level, const char* fmt, ...){
    if(level < g_logLevel) {
        return;
    }

    time_t now = time(nullptr);
    struct tm* tm_now = localtime(&now);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_now);

    const char* levelStr[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

    fprintf(stderr, "[%s] [%s]", timebuf, levelStr[level]);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");

    if(level == FATAL) {
        abort();
    }
}