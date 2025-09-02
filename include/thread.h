#ifndef THREAD_H
#define THREAD_H

#include <arena.h>
#include <pthread.h>

#include "startup.h"
#include "window.h"

#define DISPLAY_SLEEP_TIME 1000 * 100 // ui thread sleep time
#define PROC_WAIT_TIME_SEC 2
#define MIN_QUEUE_SIZE 5
#define READ_SLEEP_TIME 1000 * 200 // io thread sleep time

extern pthread_mutex_t taskQueueLock;
extern pthread_cond_t taskQueueCondition;
// I actually don't need these
// right now, but keeping around
// because I think I will soon.
extern pthread_mutex_t listStateLock;
extern pthread_cond_t listStateCondition;

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
    UIData *ui
);

//
//	io_thread.c
//
//
void run_io(
    mtopArenas *arenas,
    WindowData **windows
);
#endif
