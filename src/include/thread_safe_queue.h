#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <pthread.h>
#include <arena.h>

#include "mt_type_defs.h"

typedef struct _queue_node
{
	void *data;
	struct _queue_node *next;
} QueueNode;

typedef struct _shared_queue
{
	QueueNode *head, *tail;
	size_t size;
} ThreadSafeQueue;

void enqueue(
	ThreadSafeQueue *q,
	void *stats,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition
);
u8 dequeue(
	ThreadSafeQueue *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition
);
void * peek(
	ThreadSafeQueue *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition
);

#endif

