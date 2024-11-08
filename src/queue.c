#include <pthread.h>
#include <stdlib.h>

#include "queue.h"
#include "include/cpu_monitor.h"

void enqueue(QUEUE *q, CPU_STATS *stats, pthread_mutex_t *queueLock, pthread_cond_t *condition)
{
	NODE *newNode = malloc(sizeof(NODE));

	pthread_mutex_lock(queueLock);

	if (q->head == NULL)
	{
		q->head  = newNode;
		q->tail = q->head;
		q->head->next = NULL;
		q->head->data = stats;

		pthread_mutex_unlock(queueLock);

		return;
	}

	newNode->data = stats;

	q->tail->next = newNode;
	q->tail = newNode;
	q->tail->next = NULL;

	q->size++;

	pthread_mutex_unlock(queueLock);
}

int dequeue(QUEUE *q, pthread_mutex_t *queueLock, pthread_cond_t *condition)
{
	pthread_mutex_lock(queueLock);

	if (q->head == NULL)
	{
		pthread_mutex_unlock(queueLock);
		return 0;
	}
 
	NODE *tmp = q->head;

	q->head = q->head->next;

	free(tmp);

	q->size--;

	pthread_mutex_unlock(queueLock);

	return 1;
}

CPU_STATS * peek (QUEUE *q, pthread_mutex_t *queueLock, pthread_cond_t *condition)
{
	pthread_mutex_lock(queueLock);

	if (q->head == NULL) 
	{
		pthread_mutex_unlock(queueLock);
		return NULL;
	}

	CPU_STATS *val = q->head->data;

	pthread_mutex_unlock(queueLock);

	return val;
}
