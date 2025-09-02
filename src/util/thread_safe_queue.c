#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "../../include/thread_safe_queue.h"

void enqueue(
    ThreadSafeQueue *queue,
    void *data,
    pthread_mutex_t *queueLock,
    pthread_cond_t *condition
)
{
    pthread_mutex_lock(queueLock);
    
    QueueNode *newNode = malloc(sizeof(QueueNode));
    
    assert(newNode);
    
    if(queue->head == NULL)
    {
		queue->head = newNode;
    	queue->tail = queue->head;
    	queue->head->next = NULL;
    	queue->head->data = data;
    	
    	queue->size++;
    	
    	pthread_cond_signal(condition);
    	pthread_mutex_unlock(queueLock);
    	
    	return;
    }
    
    newNode->data = data;
    queue->tail->next = newNode;
    queue->tail = queue->tail->next;
    queue->tail->next = NULL;
    queue->size++;
    
    pthread_cond_signal(condition);
    pthread_mutex_unlock(queueLock);
}

u8 dequeue(
    ThreadSafeQueue *queue,
    pthread_mutex_t *queueLock,
    pthread_cond_t *condition
)
{
    pthread_mutex_lock(queueLock);
    
    while (queue->head == NULL)
    {
		pthread_cond_wait(condition, queueLock);
    }
    
    QueueNode *tmp = queue->head;
    
    queue->head = queue->head->next;
    
    free(tmp);
    
    tmp = NULL;
    
    queue->size--;
    
    pthread_mutex_unlock(queueLock);
    
    return 1;
}

void * peek(
    ThreadSafeQueue *queue,
    pthread_mutex_t *queueLock,
    pthread_cond_t *condition
)
{
    pthread_mutex_lock(queueLock);
    
    while (queue->head == NULL)
    {
		pthread_cond_wait(condition, queueLock);
    }
    
    void *val = queue->head->data;
    
    pthread_mutex_unlock(queueLock);
    
    return val;
}
