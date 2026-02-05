#ifndef BASE_TEST_H
#define BASE_TEST_H

#include <unistd.h>
#include "base.h"

internal void
fatal(char *msg);

#define STR_LEN(s) (sizeof(s) - 1)

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

#define show(fd) \
    do \
    { \
        write((fd), __FILE__, STR_LEN(__FILE__)); \
        write((fd), ":", 1); \
        write((fd), __func__, STR_LEN(__func__)); \
        write((fd), ":", 1); \
        write((fd), "\n", 1); \
    } while (0)

#define test(expr) \
    do \
    { \
        if ((expr) != 0) \
        { \
            write(STDERR_FILENO, "[FAILED] ", 9); \
            show(STDERR_FILENO); \
            _exit(1); \
        } \
    } while (0)

#define check(expr) \
    do \
    { \
        if ((expr) != 0) \
        { \
            write(STDERR_FILENO, RED "[ERROR] " RESET, 18); \
            show(STDERR_FILENO); \
            _exit(1); \
        } \
        else \
        { \
            write(STDERR_FILENO, GREEN "[SUCCESS] " RESET, 20); \
            show(STDERR_FILENO); \
        } \
    } while (0)

#define checkpoint \
    do \
    { \
        write(STDERR_FILENO, BLUE "<<CHECKPOINT>>\n" RESET, 27); \
        show(STDERR_FILENO); \
        write(STDERR_FILENO, BLUE "^^^^^^^^^^^^^^\n\n\n" RESET, 29); \
    } while (0)

#endif /* BASE_TEST_H */
