#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <pthread.h>
#include <arena.h>

typedef struct _queue_node
{
	void *data;
	struct _queue_node *next;
} QueueNode;

typedef struct _shared_queue
{
	QueueNode *head, *tail;
	int size;
} ThreadSafeQueue;

void enqueue(
	ThreadSafeQueue *q,
	void *stats,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition
);
int dequeue(
	ThreadSafeQueue *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition
);
void * peek(
	ThreadSafeQueue *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition,
	int wait
);

#endif

