/**
 * @file log.h
 * @date 2011-08-29
 * @author Fox (yulefox at gmail.com)
 * 封装 log4cplus, 提供写入日志的功能, 在石喜实现版本的基础上作了部分简化.
 */

#ifndef ELF_LOG_LOG_H
#define ELF_LOG_LOG_H
#pragma once

#include "public/def.h"

#ifdef _DEBUG
#pragma comment(lib, "log4cplusSD.lib")
#else
#pragma comment(lib, "log4cplusS.lib")
#endif /* _DEBUG */

#define MAX_LOG_LENGTH 2049

BEGIN_C_DECLS

/**
 * @return 0
 */
int log_init(void);

/**
 * @return 0
 */
int log_fini(void);
const char *log_fmt(char *buf, const char *fmt, ...);

END_C_DECLS

#include <log4cplus/logger.h>

#define LOG_TRACE(mod, fmt, ...) do { \
    char buf[MAX_LOG_LENGTH]; \
    log4cplus::Logger::getInstance(mod).forcedLog(log4cplus::TRACE_LOG_LEVEL, \
    log_fmt(buf, fmt, ##__VA_ARGS__), __FILE__, __LINE__); \
} while (0)

#define LOG_DEBUG(mod, fmt, ...) do { \
    char buf[MAX_LOG_LENGTH]; \
log4cplus::Logger::getInstance(mod).forcedLog(log4cplus::DEBUG_LOG_LEVEL, \
        log_fmt(buf, fmt, ##__VA_ARGS__), __FILE__, __LINE__); \
} while (0)

#define LOG_INFO(mod, fmt, ...) do { \
    char buf[MAX_LOG_LENGTH]; \
    log4cplus::Logger::getInstance(mod).forcedLog(log4cplus::INFO_LOG_LEVEL, \
            log_fmt(buf, fmt, ##__VA_ARGS__), __FILE__, __LINE__); \
} while (0)

#define LOG_WARN(mod, fmt, ...) do { \
    char buf[MAX_LOG_LENGTH]; \
    log4cplus::Logger::getInstance(mod).forcedLog(log4cplus::WARN_LOG_LEVEL, \
            log_fmt(buf, fmt, ##__VA_ARGS__), __FILE__, __LINE__); \
} while (0)

#define LOG_ERROR(mod, fmt, ...) do { \
    char buf[MAX_LOG_LENGTH]; \
    log4cplus::Logger::getInstance(mod).forcedLog(log4cplus::ERROR_LOG_LEVEL, \
            log_fmt(buf, fmt, ##__VA_ARGS__), __FILE__, __LINE__); \
} while (0)

#define LOG_FATAL(mod, fmt, ...) do { \
    char buf[MAX_LOG_LENGTH]; \
    log4cplus::Logger::getInstance(mod).forcedLog(log4cplus::FATAL_LOG_LEVEL, \
            log_fmt(buf, fmt, ##__VA_ARGS__), __FILE__, __LINE__); \
} while (0)

#endif /* !ELF_LOG_LOG_H */
