/*
 * Copyright (c) 2018-2021 Qinglong<sysu.zqlong@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "osal/os_log.h"

#if defined(OS_ANDROID)
#include <android/log.h>
void os_fatal(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    __android_log_print(ANDROID_LOG_FATAL, tag, format, arg_ptr);
    va_end(arg_ptr);
}
void os_error(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    __android_log_print(ANDROID_LOG_ERROR, tag, format, arg_ptr);
    va_end(arg_ptr);
}
void os_warning(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    __android_log_print(ANDROID_LOG_WARN, tag, format, arg_ptr);
    va_end(arg_ptr);
}
void os_info(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    __android_log_print(ANDROID_LOG_INFO, tag, format, arg_ptr);
    va_end(arg_ptr);
}
void os_debug(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    __android_log_print(ANDROID_LOG_DEBUG, tag, format, arg_ptr);
    va_end(arg_ptr);
}
void os_verbose(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    __android_log_print(ANDROID_LOG_VERBOSE, tag, format, arg_ptr);
    va_end(arg_ptr);
}

#else
#include <string.h>
#include <stdarg.h>
#include "osal/os_time.h"

#define LOG_BUFFER_SIZE  1024

enum log_level {
    LOG_FATAL = 0,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
    LOG_VERBOSE,
};

static const char *log_level_strings[] = {
    [LOG_FATAL]   = "F",
    [LOG_ERROR]   = "E",
    [LOG_WARN]    = "W",
    [LOG_INFO]    = "I",
    [LOG_DEBUG]   = "D",
    [LOG_VERBOSE] = "V",
};

// [date] [time] [prio] [tag]: [log]
static void log_print(enum log_level prio, const char *tag, const char *format, va_list arg_ptr);

void os_fatal(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    log_print(LOG_FATAL, tag, format, arg_ptr);
    va_end(arg_ptr);
}
void os_error(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    log_print(LOG_ERROR, tag, format, arg_ptr);
    va_end(arg_ptr);
}
void os_warning(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    log_print(LOG_WARN, tag, format, arg_ptr);
    va_end(arg_ptr);
}
void os_info(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    log_print(LOG_INFO, tag, format, arg_ptr);
    va_end(arg_ptr);
}
void os_debug(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    log_print(LOG_DEBUG, tag, format, arg_ptr);
    va_end(arg_ptr);
}
void os_verbose(const char *tag, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    log_print(LOG_VERBOSE, tag, format, arg_ptr);
    va_end(arg_ptr);
}

static void log_print(enum log_level prio, const char *tag, const char *format, va_list arg_ptr)
{
    size_t offset = 0;
    int arg_size = 0;
    char log_entry[LOG_BUFFER_SIZE];
    size_t valid_size = LOG_BUFFER_SIZE - 2;

    struct os_wall_time ts;
    // add data & time to header
    os_realtime_to_walltime(&ts);
    if ((int)(valid_size - offset) > 0)
        offset += snprintf(log_entry + offset, valid_size - offset,
                           "%4d-%02d-%02d %02d:%02d:%02d:%03d",
                           ts.year, ts.mon, ts.day, ts.hour, ts.min, ts.sec, ts.msec);

    // add priority to header
    if ((int)(valid_size - offset) > 0)
        offset += snprintf(log_entry + offset, valid_size - offset,
                           " %s", log_level_strings[prio]);

    // add tag to header
    if ((int)(valid_size - offset) > 0)
        offset += snprintf(log_entry + offset, valid_size - offset, " %s: ", tag);

    if ((int)(valid_size - offset) > 0) {
        arg_size = vsnprintf(log_entry + offset, valid_size - offset, format, arg_ptr);
        if (arg_size > 0) {
            offset += arg_size;
            if (offset > valid_size)
                offset = valid_size - 1;
        }
        log_entry[offset++] = '\n';
        log_entry[offset] = '\0';
    } else {
        offset = valid_size - 1;
        log_entry[offset++] = '\n';
        log_entry[offset] = '\0';
    }

    // print log to console
    fprintf(stdout, "%s", log_entry);
}
#endif // !OS_ANDROID