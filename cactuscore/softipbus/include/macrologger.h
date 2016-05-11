/*
 * Copyright (c) 2012 David Rodrigues
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __MACROLOGGER_H__
#define __MACROLOGGER_H__

#include <time.h>
#include <string.h>
#include <unistd.h>

// === auxiliar functions
static inline char *timenow();

#define _FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#define NO_LOG          0x00
#define ERROR_LEVEL     0x01
#define INFO_LEVEL      0x02
#define DEBUG_LEVEL     0x03

#ifndef LOG_LEVEL
#define LOG_LEVEL   DEBUG_LEVEL
#endif

#if (defined __linux__ || defined __APPLE__)
#include <stdio.h>
#define PRINTFUNCTION(format, ...)      fprintf(stderr, format, __VA_ARGS__)
#else
#include "xil_printf.h"
#define PRINTFUNCTION(format, ...)      xil_printf(format, __VA_ARGS__)
#endif

//#define LOG_FMT             "%i | %s | %-5s | %15s:%-25s | "
//#define LOG_ARGS(LOG_TAG)   getpid(), timenow(), LOG_TAG, _FILE, __FUNCTION__
//#define LOG_FMT             "%s | %-5s | "
//#define LOG_ARGS(LOG_TAG)   timenow(), LOG_TAG
#define LOG_FMT             "%s: "
#define LOG_ARGS(LOG_TAG)   LOG_TAG

#define NEWLINE     "\n"

#define ERROR_TAG   "ERROR"
#define INFO_TAG    "INFO"
#define DEBUG_TAG   "DEBUG"

#if LOG_LEVEL >= DEBUG_LEVEL
#define LOG_DEBUG(message, ...)     PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(DEBUG_TAG), ## __VA_ARGS__)
#else
#define LOG_DEBUG(message, ...)
#endif

#if LOG_LEVEL >= INFO_LEVEL
#define LOG_INFO(message, ...)      PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(INFO_TAG), ## __VA_ARGS__)
#else
#define LOG_INFO(message, ...)
#endif

#if LOG_LEVEL >= ERROR_LEVEL
#define LOG_ERROR(message, ...)     PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(ERROR_TAG), ## __VA_ARGS__)
#else
#define LOG_ERROR(message, ...)
#endif

#if LOG_LEVEL >= NO_LOGS
#define LOG_IF_ERROR(condition, message, ...) if (condition) PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(ERROR_TAG), ## __VA_ARGS__)
#else
#define LOG_IF_ERROR(condition, message, ...)
#endif

static inline char *timenow() {
    static char buffer[64];
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);

    return buffer;
}

#endif
