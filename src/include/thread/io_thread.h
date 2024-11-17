#ifndef IO_THREAD_H
#define IO_THREAD_H

#include <pthread.h>
#include <arena.h>

#include "../util/shared_queue.h"
#include "../monitor/proc_monitor.h"

#define PROC_WAIT_TIME_SEC 5
#define MIN_QUEUE_SIZE 5
#define READ_SLEEP_TIME 1000 * 100 

void run_io(
	Arena *cpuArena,
	Arena *memArena,
	Arena *procArena,
	SHARED_QUEUE *cpuQueue,
	SHARED_QUEUE *memQueue
);

#endif
