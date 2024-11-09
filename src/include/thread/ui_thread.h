#ifndef UI_THREAD_H
#define UI_THREAD_H

#include <arena.h>
#include <pthread.h>

#include "../util/shared_queue.h"
#include "../window/window.h"


void run_graphs(
	Arena *graphArena,
	Arena *memGraphArena,
	WINDOW_DATA *cpuWin,
	WINDOW_DATA *memWin,
	SHARED_QUEUE *cpuQueue,
	SHARED_QUEUE *memoryQueue	
);

#endif
