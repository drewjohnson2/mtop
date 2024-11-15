#include "../include/thread/thread.h"

pthread_mutex_t runLock;
pthread_mutex_t cpuQueueLock;
pthread_mutex_t memQueueLock;
pthread_mutex_t procQueueLock;
                                 
pthread_cond_t cpuQueueCondition;
pthread_cond_t memQueueCondition;
pthread_cond_t procQueueCondition;

volatile int SHUTDOWN_FLAG = 0;

void mutex_init()
{
	pthread_mutex_init(&runLock, NULL);
	pthread_mutex_init(&cpuQueueLock, NULL);
	pthread_mutex_init(&memQueueLock, NULL);
	pthread_mutex_init(&procQueueLock, NULL);
}

void condition_init()
{
	pthread_cond_init(&cpuQueueCondition, NULL);
	pthread_cond_init(&memQueueCondition, NULL);
	pthread_cond_init(&procQueueCondition, NULL);
}

void mutex_destroy()
{
	pthread_mutex_destroy(&runLock);
	pthread_mutex_destroy(&cpuQueueLock);
	pthread_mutex_destroy(&memQueueLock);
	pthread_mutex_destroy(&procQueueLock);
}

void condition_destroy()
{
	pthread_cond_destroy(&cpuQueueCondition);
	pthread_cond_destroy(&memQueueCondition);
	pthread_cond_destroy(&procQueueCondition);
}
