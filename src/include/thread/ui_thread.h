#ifndef UI_THREAD_H
#define UI_THREAD_H

#include <arena.h>
#include <pthread.h>

#include "../util/shared_queue.h"
#include "../window/window.h"
#include "../monitor/proc_monitor.h"

#define DISPLAY_SLEEP_TIME 1000 * 200

void run_graphs(
	Arena *graphArena,
	Arena *memGraphArena,
	DISPLAY_ITEMS *di,
	SHARED_QUEUE *cpuQueue,
	SHARED_QUEUE *memoryQueue,
	PROC_STATS **procStats
);

#endif
