#ifndef THREAD_H
#define THREAD_H

#include <arena.h>
#include <pthread.h>

#include "thread_safe_queue.h"
#include "window.h"

#define DISPLAY_SLEEP_TIME 1000 * 200 
#define PROC_WAIT_TIME_SEC 2
#define MIN_QUEUE_SIZE 5
#define READ_SLEEP_TIME 1000 * 100 


#define SHOULD_MERGE(mutex, cont) 		\
    do { 					\
	switch (pthread_mutex_trylock(mutex)) 	\
    	{ 					\
	    case 0: 				\
	        pthread_mutex_unlock(mutex); 	\
	        cont = 0; 			\
	        break; 				\
    	    case EBUSY: 			\
	       break; 				\
    	} 					\
    } while(0) 					\

extern pthread_mutex_t cpuQueueLock;
extern pthread_mutex_t memQueueLock;
extern pthread_mutex_t procDataLock;

extern pthread_cond_t cpuQueueCondition;
extern pthread_cond_t memQueueCondition;
extern pthread_cond_t procQueueCondition;

extern volatile int SHUTDOWN_FLAG;

//
// 		thread.c
//
//
void mutex_init();
void condition_init();
void mutex_destroy();
void condition_destroy();

//
//		ui_thread.c
//
//
void run_ui(
    Arena *graphArena,
    Arena *memGraphArena,
    Arena *procArena,
    DisplayItems *di,
    ThreadSafeQueue *cpuQueue,
    ThreadSafeQueue *memoryQueue,
    ThreadSafeQueue *procQueue
);

//
//		io_thread.c
//
//
void run_io(
    Arena *cpuArena,
    Arena *memArena,
    Arena *procArena,
    ThreadSafeQueue *cpuQueue,
    ThreadSafeQueue *memQueue,
    ThreadSafeQueue *procQueue
);
#endif
