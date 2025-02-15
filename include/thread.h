#ifndef THREAD_H
#define THREAD_H

#include <arena.h>
#include <pthread.h>

#include "monitor.h"
#include "thread_safe_queue.h"
#include "window.h"

#define DISPLAY_SLEEP_TIME 1000 * 200 // ui thread sleep time
#define PROC_WAIT_TIME_SEC 2
#define MIN_QUEUE_SIZE 5
#define READ_SLEEP_TIME 1000 * 100 // io thread sleep time

extern pthread_mutex_t cpuQueueLock;
extern pthread_mutex_t memQueueLock;
extern pthread_mutex_t procDataLock;
extern pthread_mutex_t procInfoLock;

extern pthread_cond_t cpuQueueCondition;
extern pthread_cond_t memQueueCondition;
extern pthread_cond_t procQueueCondition;
extern pthread_cond_t procInfoCondition;

extern volatile s8 SHUTDOWN_FLAG;

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
    DisplayItems *di,
    ThreadSafeQueue *cpuQueue,
    ThreadSafeQueue *memoryQueue,
    ThreadSafeQueue *procQueue,
    volatile ProcessInfoSharedData *prcInfoSd
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
    ThreadSafeQueue *procQueue,
    volatile ProcessInfoSharedData *prcInfoSd
);
#endif
