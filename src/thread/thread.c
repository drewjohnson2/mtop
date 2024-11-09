#include "../include/thread/thread.h"

pthread_mutex_t runLock;
pthread_mutex_t cpuQueueLock;
pthread_mutex_t memQueueLock;
                                 
pthread_cond_t cpuQueueCondition;
pthread_cond_t memQueueCondition;

void mutex_init()
{
	pthread_mutex_init(&runLock, NULL);
	pthread_mutex_init(&cpuQueueLock, NULL);
	pthread_mutex_init(&memQueueLock, NULL);
}

void condition_init()
{
	pthread_cond_init(&cpuQueueCondition, NULL);
	pthread_cond_init(&memQueueCondition, NULL);
}

void mutex_destroy()
{
	pthread_mutex_destroy(&runLock);
	pthread_mutex_destroy(&cpuQueueLock);
	pthread_mutex_destroy(&memQueueLock);
}

void condition_destroy()
{
	pthread_cond_destroy(&cpuQueueCondition);
	pthread_cond_destroy(&memQueueCondition);
}
