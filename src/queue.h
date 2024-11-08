#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

#include "include/cpu_monitor.h"

typedef struct _q_node 
{
	CPU_STATS *data;
	struct _q_node *next;
} NODE;

typedef struct _queue
{
	NODE *head, *tail;
	int size;
} QUEUE;

void enqueue(QUEUE *q, CPU_STATS *stats, pthread_mutex_t *queueLock, pthread_cond_t *condition);
int dequeue(QUEUE *q, pthread_mutex_t *queueLock, pthread_cond_t *condition);
CPU_STATS * peek(QUEUE *q, pthread_mutex_t *queueLock, pthread_cond_t *condition);

#endif

