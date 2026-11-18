
#ifndef LOG_H
#define LOG_H

#include "stdio.h"
// Default log level if not specified by compiler
#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif

#define LOG_ERROR(fmt, ...) \
    do { \
        if (LOG_LEVEL >= 1) \
            printf("[ERROR] " fmt "\n", ##__VA_ARGS__); \
    } while (0)

#define LOG_INFO(fmt, ...) \
    do { \
        if (LOG_LEVEL >= 2) \
            printf("[INFO ] " fmt "\n", ##__VA_ARGS__); \
    } while (0)

#define LOG_DEBUG(fmt, ...) \
    do { \
        if (LOG_LEVEL >= 3) \
            printf("[DEBUG] " fmt "\n", ##__VA_ARGS__); \
    } while (0)

#endif
