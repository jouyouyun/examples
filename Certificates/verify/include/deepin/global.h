#pragma once

#include <stdio.h>

enum LOG_TYPE {
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
};

static const char *log_level_str[] = {
    [LOG_ERROR] = "ERROR",
    [LOG_WARN] = "WARN",
    [LOG_INFO] = "INFO",
    [LOG_DEBUG] = "DEBUG",
};

#ifdef DEEPIN_MSG_DEBUG
	#define LOG_LEVEL LOG_DEBUG
#else
	#define LOG_LEVEL LOG_INFO
#endif

#define deepin_message(level, fmt, ...) do {\
        if (LOG_LEVEL >= level) {\
            fprintf(stderr, "[%s] " fmt, log_level_str[level], ##__VA_ARGS__);\
        }\
} while(0)
