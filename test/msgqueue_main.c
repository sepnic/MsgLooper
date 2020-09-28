#include <stdio.h>
#include <string.h>
#include "cutils/os_memory.h"
#include "cutils/os_logger.h"
#include "cutils/os_thread.h"
#include "cutils/msgqueue.h"

#define LOG_TAG "msgqueue_test"

#define STR_LENGTH          32

#define QUEUE_1_LENGTH      10
#define QUEUE_2_LENGTH      10

#define QUEUE_1_ITEM_SIZE   sizeof(unsigned int)
#define QUEUE_2_ITEM_SIZE   STR_LENGTH

#define QUEUE_SET_LENGTH    (QUEUE_1_LENGTH + QUEUE_2_LENGTH)

static mqueueset_t set = NULL;
static mqueue_t queue1 = NULL;
static mqueue_t queue2 = NULL;

static void *queue_send_thread(void *arg)
{
    char str[STR_LENGTH];
    memset(str, 0x0, sizeof(str));

    for (int i = 0; i < 20; i++) {
        snprintf(str, sizeof(str), "->%d", i);
        mqueue_send(queue1, (char *)&i, 100);
        mqueue_send(queue2, str, 0);
    }

    return NULL;
}

int main()
{
    set = mqueueset_create(QUEUE_SET_LENGTH);
    queue1 = mqueue_create(QUEUE_1_ITEM_SIZE, QUEUE_1_LENGTH);
    queue2 = mqueue_create(QUEUE_2_ITEM_SIZE, QUEUE_2_LENGTH);
    if (set == NULL || queue1 == NULL || queue2 == NULL) {
        OS_LOGE(LOG_TAG, "Failed to allocate queue");
        goto error;
    }

    mqueueset_add_queue(set, queue1);
    mqueueset_add_queue(set, queue2);

    {
        struct os_threadattr attr = {
            .name = "queue_send",
            .priority = OS_THREAD_PRIO_NORMAL,
            .stacksize = 1024,
            .joinable = false,
        };
        OS_THREAD_CREATE(&attr, queue_send_thread, NULL);
    }

    while (true) {
        mqueue_t active_queue = NULL;
        unsigned int queue1_item;
        char queue2_item[STR_LENGTH];
        int ret;

        //OS_LOGI(LOG_TAG, "Waiting msg sent to set");
        active_queue = mqueueset_select_queue(set, 1000);
        if (active_queue == queue1) {
            ret = mqueue_receive(queue1, (char *)&queue1_item, 0);
            if (ret == 0)
                OS_LOGI(LOG_TAG, "Succeed to receive msg=[%d] from queue1", queue1_item);
            else
                OS_LOGE(LOG_TAG, "Failed to receive msg from queue1");
        }
        else if (active_queue == queue2) {
            memset(queue2_item, 0x0, sizeof(queue2_item));
            ret = mqueue_receive(queue2, (char *)&queue2_item, 0);
            if (ret == 0)
                OS_LOGI(LOG_TAG, "Succeed to receive msg=[%s] from queue2", queue2_item);
            else
                OS_LOGE(LOG_TAG, "Failed to receive msg from queue2");
        }
        else if (active_queue == NULL) {
            OS_LOGE(LOG_TAG, "Timeout to obtain queue from set");
            break;
        }
    }

error:
    if (set != NULL)
        mqueueset_destroy(set);
    if (queue1 != NULL)
        mqueue_destroy(queue1);
    if (queue2 != NULL)
        mqueue_destroy(queue2);
    return 0;
}
