#include <assert.h>
#include <pthread.h>

#include "../../include/thread.h"

pthread_mutex_t taskQueueLock;
pthread_cond_t taskQueueCondition;
pthread_mutex_t listStateLock;
pthread_cond_t listStateCondition;

volatile s8 SHUTDOWN_FLAG = 0;

void mutex_init()
{
    pthread_mutex_init(&taskQueueLock, NULL);
    pthread_mutex_init(&listStateLock, NULL);
    pthread_cond_init(&listStateCondition, NULL);
    pthread_cond_init(&taskQueueCondition, NULL);
    
    assert(&taskQueueLock);
    assert(&listStateLock);
    assert(&taskQueueCondition);
    assert(&listStateCondition);
}

void mutex_destroy()
{
    pthread_mutex_destroy(&taskQueueLock);
    pthread_mutex_destroy(&listStateLock);
    pthread_cond_destroy(&taskQueueCondition);
    pthread_cond_destroy(&listStateCondition);
}
