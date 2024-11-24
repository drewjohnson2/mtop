#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "../include/thread_safe_queue.h"

void enqueue(ThreadSafeQueue *q, void *stats, pthread_mutex_t *queueLock, pthread_cond_t *condition)
{
	pthread_mutex_lock(queueLock);

	QueueNode *newNode = malloc(sizeof(QueueNode));

	assert(newNode);

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

int dequeue(
	ThreadSafeQueue *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition
)
{
	pthread_mutex_lock(queueLock);

	while (q->head == NULL)
	{
		pthread_cond_wait(condition, queueLock);
	}

	QueueNode *tmp = q->head;

	q->head = q->head->next;

	free(tmp);

	tmp = NULL;

	q->size--;

	pthread_mutex_unlock(queueLock);

	return 1;
}

void * peek(
	ThreadSafeQueue *q,
	pthread_mutex_t *queueLock,
	pthread_cond_t *condition,
	int wait
)
{
	pthread_mutex_lock(queueLock);

	while (q->head == NULL)
	{
		if (!wait)
		{
			pthread_mutex_unlock(queueLock);
			return NULL;
		}

		pthread_cond_wait(condition, queueLock);
	}

	void *val = q->head->data;

	pthread_mutex_unlock(queueLock);

	return val;
}
