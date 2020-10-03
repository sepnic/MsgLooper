/*
 * Copyright (C) 2018-2020 luoyun <sysu.zqlong@gmail.com>
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

#include <string.h>
#include "cutils/os_memory.h"
#include "cutils/os_timer.h"

#if defined(OS_LINUX) || defined(OS_ANDROID)
#include <unistd.h>
#include <time.h>
#include <signal.h>

typedef void (*timer_notify_cb)(union sigval val);

struct linux_timer {
    timer_t id;
    unsigned long period_ms;
    bool reload;
    bool started;
};

os_timer_t OS_TIMER_CREATE(struct os_timerattr *attr, void (*cb)())
{
    struct linux_timer *handle = NULL;
    timer_t id;
    struct sigevent ent;
    int ret;

    memset(&ent, 0x00, sizeof(struct sigevent));
    ent.sigev_notify = SIGEV_THREAD;
    ent.sigev_notify_function = (timer_notify_cb)cb;
    ret = timer_create(CLOCK_MONOTONIC, &ent, &id);
    if (ret != 0)
        return NULL;

    handle = OS_MALLOC(sizeof(struct linux_timer));
    if (handle != NULL) {
        handle->id = id;
        handle->period_ms = attr->period_ms;
        handle->reload = attr->reload;
        handle->started = false;
    }
    else {
        timer_delete(id);
    }

    return (os_timer_t)handle;
}

int OS_TIMER_START(os_timer_t timer)
{
    struct linux_timer *handle = (struct linux_timer *)timer;
    struct itimerspec value;
    int ret;

    memset(&value, 0x00, sizeof(struct itimerspec));
    value.it_value.tv_sec = handle->period_ms / 1000;
    value.it_value.tv_nsec = (handle->period_ms % 1000) * 1000000;
    if (handle->reload) {
        value.it_interval.tv_sec = value.it_value.tv_sec;
        value.it_interval.tv_nsec = value.it_value.tv_nsec;
    }

    ret = timer_settime(handle->id, 0, &value, NULL);
    if (ret == 0)
        handle->started = true;

    return ret;
}

int OS_TIMER_STOP(os_timer_t timer)
{
    struct linux_timer *handle = (struct linux_timer *)timer;
    struct itimerspec value;
    int ret;

    memset(&value, 0x00, sizeof(struct itimerspec));

    ret = timer_settime(handle->id, 0, &value, NULL);
    if (ret == 0)
        handle->started = false;

    return ret;
}

bool OS_TIMER_IS_ACTIVE(os_timer_t timer)
{
    struct linux_timer *handle = (struct linux_timer *)timer;
    struct itimerspec value;

    if (!handle->started)
        return false;

    if (handle->reload) {
        return true;
    }
    else {
        memset(&value, 0x00, sizeof(struct itimerspec));
        timer_gettime(handle->id, &value);
        return value.it_value.tv_sec != 0 || value.it_value.tv_nsec != 0;
    }
}

void OS_TIMER_DESTROY(os_timer_t timer)
{
    struct linux_timer *handle = (struct linux_timer *)timer;

    timer_delete(handle->id);
    OS_FREE(handle);
}

#elif defined(OS_MACOSX) || defined(OS_IOS) || defined(OS_FREERTOS)

#include "cutils/sw_timer.h"

os_timer_t OS_TIMER_CREATE(struct os_timerattr *attr, void (*cb)())
{
    struct swtimer_attr cattr;
    swtimer_t timer;

    cattr.name = attr->name;
    cattr.period_ms = attr->period_ms;
    cattr.reload = attr->reload;
    timer = swtimer_create(&cattr, cb);

    return (os_timer_t)timer;
}

int OS_TIMER_START(os_timer_t timer)
{
    return swtimer_start((swtimer_t)timer);
}

int OS_TIMER_STOP(os_timer_t timer)
{
    return swtimer_stop((swtimer_t)timer);
}

bool OS_TIMER_IS_ACTIVE(os_timer_t timer)
{
    return swtimer_is_active((swtimer_t)timer);
}

void OS_TIMER_DESTROY(os_timer_t timer)
{
    swtimer_destroy((swtimer_t)timer);
}

#endif
