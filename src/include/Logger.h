#pragma once

#include <cstdarg>

#ifndef LOG_LEVEL
#define LOG_LEVEL 4
#endif

enum LogLevel
{
	LEVEL_FATAL = 0,
	LEVEL_ERROR = 1,
	LEVEL_WARN = 2,
	LEVEL_INFO = 3,
	LEVEL_DEBUG = 4,
	LEVEL_TRACE = 5
};

void log(LogLevel logLevel, const char* message, ...);


#if LOG_LEVEL > 0
#define LOG_FATAL(message,  ...) log(LEVEL_FATAL, message, __VA_ARGS__);
#define LOG_ERROR(message,  ...) log(LEVEL_ERROR, message, __VA_ARGS__);
#else
#define LOG_FATAL(message, ...)
#define LOG_ERROR(message, ...)
#endif

#if LOG_LEVEL > 1
#define LOG_WARN(message, ...) log(LEVEL_WARN, message, __VA_ARGS__);
#else
#define LOG_WARN(message, ...)
#endif

#if LOG_LEVEL > 2
#define LOG_INFO(message, ...) log(LEVEL_INFO, message, __VA_ARGS__);
#else
#define LOG_INFO(message, ...)
#endif

#if LOG_LEVEL > 3
#define LOG_DEBUG(message, ...) log(LEVEL_DEBUG, message, __VA_ARGS__);
#else
#define LOG_DEBUG(message, ...)
#endif

#if LOG_LEVEL > 4
#define LOG_TRACE(message, ...) log(LEVEL_TRACE, message, __VA_ARGS__);
#else
#define LOG_TRACE(message, ...)
#endif