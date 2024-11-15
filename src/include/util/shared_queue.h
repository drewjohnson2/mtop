#ifndef SHARED_QUEUE_H
#define SHARED_QUEUE_H

#include <pthread.h>

typedef struct _queue_node
{
	void *data;
	struct _queue_node *next;
} QUEUE_NODE;

typedef struct _shared_queue
{
	QUEUE_NODE *head, *tail;
	int size;
} SHARED_QUEUE;

void enqueue(
	SHARED_QUEUE *q,
	void *stats,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition
);
int dequeue(
	SHARED_QUEUE *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition
);
void * peek(
	SHARED_QUEUE *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition
);
int timedDequeue(
	SHARED_QUEUE *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition,
	unsigned short timeoutSec
);
void * timedPeek(
	SHARED_QUEUE *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition,
	unsigned short timeoutSec
);

#endif

