#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

#define SHOULD_MERGE(mutex, cont) \
	do { \
		switch (pthread_mutex_trylock(mutex)) \
		{ \
			case 0: \
				pthread_mutex_unlock(mutex); \
				cont = 0; \
				break; \
			case EBUSY: \
				break; \
		} \
	} while(0) \

extern pthread_mutex_t runLock;
extern pthread_mutex_t cpuQueueLock;
extern pthread_mutex_t memQueueLock;

extern pthread_cond_t cpuQueueCondition;
extern pthread_cond_t memQueueCondition;

void mutex_init();
void condition_init();
void mutex_destroy();
void condition_destroy();

#endif
