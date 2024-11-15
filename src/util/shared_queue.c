#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "../include/util/shared_queue.h"

void enqueue(SHARED_QUEUE *q, void *stats, pthread_mutex_t *queueLock, pthread_cond_t *condition)
{
	pthread_mutex_lock(queueLock);

	QUEUE_NODE *newNode = malloc(sizeof(QUEUE_NODE));

	if(q->head == NULL)
	{
		q->head = newNode;
		q->tail = q->head;
		q->head->next = NULL;
		q->head->data = stats;

		q->size++;

		pthread_cond_signal(condition);
		pthread_mutex_unlock(queueLock);

		return;
	}

	newNode->data = stats;

	q->tail->next = newNode;
	q->tail = q->tail->next;
	q->tail->next = NULL;

	q->size++;

	pthread_cond_signal(condition);
	pthread_mutex_unlock(queueLock);
}

int dequeue(SHARED_QUEUE *q, pthread_mutex_t *queueLock, pthread_cond_t *condition)
{
	pthread_mutex_lock(queueLock);

	while (q->head == NULL)
	{
		pthread_cond_wait(condition, queueLock);
	}

	QUEUE_NODE *tmp = q->head;

	q->head = q->head->next;

	free(tmp);

	tmp = NULL;

	q->size--;

	pthread_mutex_unlock(queueLock);

	return 1;
}

void * peek(SHARED_QUEUE *q, pthread_mutex_t *queueLock, pthread_cond_t *condition)
{
	pthread_mutex_lock(queueLock);

	while (q->head == NULL)
	{
		pthread_cond_wait(condition, queueLock);
	}

	void *val = q->head->data;

	pthread_mutex_unlock(queueLock);

	return val;
}

int timedDequeue(
	SHARED_QUEUE *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition,
	unsigned short timeoutSec
)
{
	pthread_mutex_lock(queueLock);

	if (q->head == NULL)
	{
		struct timespec start;

		clock_gettime(CLOCK_REALTIME, &start);

		start.tv_sec += timeoutSec;

		int res = pthread_cond_timedwait(condition, queueLock, &start);

		if (res == ETIMEDOUT) 
		{
			pthread_mutex_unlock(queueLock);
			return 0;
		}
	}

	QUEUE_NODE *tmp = q->head;

	q->head = q->head->next;

	free(tmp);

	tmp = NULL;

	q->size--;

	pthread_mutex_unlock(queueLock);

	return 1;
}

void * timedPeek(
	SHARED_QUEUE *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition,
	unsigned short timeoutSec
)
{
	pthread_mutex_lock(queueLock);

	if (q->head == NULL)
	{
		struct timespec start;

		clock_gettime(CLOCK_REALTIME, &start);

		start.tv_sec += timeoutSec;

		int res = pthread_cond_timedwait(condition, queueLock, &start);

		if (res == ETIMEDOUT) 
		{
			pthread_mutex_unlock(queueLock);
			return NULL;
		}	
	}

	void *val = q->head->data;

	pthread_mutex_unlock(queueLock);

	return val;
}

