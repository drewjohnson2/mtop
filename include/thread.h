#ifndef THREAD_H
#define THREAD_H

#include <arena.h>
#include <pthread.h>

#include "startup.h"
#include "thread_safe_queue.h"
#include "window.h"

#define DISPLAY_SLEEP_TIME 1000 * 200 // ui thread sleep time
#define PROC_WAIT_TIME_SEC 2
#define MIN_QUEUE_SIZE 5
#define READ_SLEEP_TIME 1000 * 100 // io thread sleep time

extern pthread_mutex_t cpuQueueLock;
extern pthread_cond_t cpuQueueCondition;

extern volatile s8 SHUTDOWN_FLAG;

//
//	thread.c
//
//
void mutex_init();
void mutex_destroy();

//
//	ui_thread.c
//
//
void run_ui(
    DisplayItems *di,
    ThreadSafeQueue *cpuQueue
);

//
//	io_thread.c
//
//
void run_io(
    mtopArenas *arenas,
    ThreadSafeQueue *cpuQueue,
    WindowData **windows
);
#endif
