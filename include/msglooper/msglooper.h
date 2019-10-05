/* The MIT License (MIT)
 *
 * Copyright (c) 2019 luoyun <sysu.zqlong@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __THREAD_LOOPER_H__
#define __THREAD_LOOPER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "os_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msglooper *looper_t;
struct message;

enum message_status {
    MESSAGE_NEW = 0,
    MESSAGE_PENDING,
    MESSAGE_RUNNING,
    MESSAGE_COMPLETED,
    MESSAGE_TIMEOUT,
    MESSAGE_DESTROY,
};

typedef void (*message_handle_cb)(struct message *msg); // handle callback
typedef void (*message_free_cb)(struct message *msg);   // free callback to free msg->data
typedef void (*message_notify_cb)(struct message *msg, enum message_status status); // notify callback
typedef bool (*message_match_cb)(struct message *msg);  // match callback

/** Please use message_obtain()/message_obtain2() to allocate a message
 *  Note: real size is sizeof(struct message_node)
 *
 * struct message *message_obtain(int what, int arg1, int arg2, void *data)
 * {
 *     struct message *msg = OS_CALLOC(1, sizeof(struct message_node));
 *     if (!msg) {
 *         OS_LOGE(LOG_TAG, "Failed to allocate message");
 *         return NULL;
 *     }
 *
 *     msg->what = what;
 *     msg->arg1 = arg1;
 *     msg->arg2 = arg2;
 *     msg->data = data;
 *     return msg;
 * }
 */
struct message {
    int what;
    int arg1;
    int arg2;
    void *data;
    unsigned long timeout_ms; // 0: means never timeout

    message_handle_cb handle_cb;
    message_free_cb free_cb;
    message_notify_cb notify_cb;
};

struct message *message_obtain(int what, int arg1, int arg2, void *data);
struct message *message_obtain2(int what, int arg1, int arg2, void *data, unsigned long timeout_ms,
                                message_handle_cb handle_cb, message_free_cb free_cb, message_notify_cb notify_cb);

looper_t looper_create(struct os_threadattr *attr, message_handle_cb handle_cb, message_free_cb free_cb);
void looper_destroy(looper_t looper);

int looper_start(looper_t looper);
void looper_stop(looper_t looper);

size_t looper_message_count(looper_t looper);
void looper_dump(looper_t looper);

int looper_enable_watchdog(looper_t looper, unsigned long long timeout_ms, void (*timeout_cb)(void *arg), void *arg);
void looper_disable_watchdog(looper_t looper);

int looper_post_message(looper_t looper, struct message *msg);
int looper_post_message_front(looper_t looper, struct message *msg);
int looper_post_message_delay(looper_t looper, struct message *msg, unsigned long msec);

int looper_remove_message(looper_t looper, int what);
int looper_remove_message_if(looper_t looper, message_match_cb match_cb);

#ifdef __cplusplus
}
#endif

#endif /* __THREAD_LOOPER_H__ */
