#include <assert.h>

#include "../../include/thread.h"

pthread_mutex_t cpuQueueLock;
pthread_mutex_t memQueueLock;
pthread_mutex_t procDataLock;
pthread_mutex_t procInfoLock;

pthread_cond_t cpuQueueCondition;
pthread_cond_t memQueueCondition;
pthread_cond_t procQueueCondition;
pthread_cond_t procInfoCondition;

volatile s8 SHUTDOWN_FLAG = 0;

void mutex_init()
{
    pthread_mutex_init(&cpuQueueLock, NULL);
    pthread_mutex_init(&memQueueLock, NULL);
    pthread_mutex_init(&procDataLock, NULL);
    pthread_mutex_init(&procInfoLock, NULL);
    
    assert(&cpuQueueLock);
    assert(&memQueueLock);
    assert(&procDataLock);
    assert(&procInfoLock);
}

void condition_init()
{
    pthread_cond_init(&cpuQueueCondition, NULL);
    pthread_cond_init(&memQueueCondition, NULL);
    pthread_cond_init(&procQueueCondition, NULL);
    pthread_cond_init(&procInfoCondition, NULL);
    
    assert(&cpuQueueCondition);
    assert(&memQueueCondition);
    assert(&procQueueCondition);
}

void mutex_destroy()
{
    pthread_mutex_destroy(&cpuQueueLock);
    pthread_mutex_destroy(&memQueueLock);
    pthread_mutex_destroy(&procDataLock);
    pthread_mutex_destroy(&procInfoLock);
}

void condition_destroy()
{
    pthread_cond_destroy(&cpuQueueCondition);
    pthread_cond_destroy(&memQueueCondition);
    pthread_cond_destroy(&procQueueCondition);
    pthread_cond_destroy(&procInfoCondition);
}
