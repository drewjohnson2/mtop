#include <assert.h>

#include "../../include/thread.h"

pthread_mutex_t cpuQueueLock;
pthread_cond_t cpuQueueCondition;

volatile s8 SHUTDOWN_FLAG = 0;

void mutex_init()
{
    pthread_mutex_init(&cpuQueueLock, NULL);
    pthread_cond_init(&cpuQueueCondition, NULL);
    
    assert(&cpuQueueLock);
    assert(&cpuQueueCondition);
}

void mutex_destroy()
{
    pthread_mutex_destroy(&cpuQueueLock);
    pthread_cond_destroy(&cpuQueueCondition);
}
