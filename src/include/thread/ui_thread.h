#ifndef UI_THREAD_H
#define UI_THREAD_H

#include <arena.h>
#include <pthread.h>

#include "../util/shared_queue.h"
#include "../window/window.h"


void run_graphs(
	Arena *graphArena,
	Arena *memGraphArena,
	DISPLAY_ITEMS *di,
	SHARED_QUEUE *cpuQueue,
	SHARED_QUEUE *memoryQueue	
);

#endif
