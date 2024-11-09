#ifndef UI_THREAD_H
#define UI_THREAD_H

#include <arena.h>
#include <pthread.h>

#include "../util/shared_queue.h"
#include "../window/window.h"


void run_cpu_graph(
	Arena *graphArena,
	WINDOW_DATA *cpuWin,
	SHARED_QUEUE *queue
);

void run_memory_graph(
	Arena *graphArena,
	WINDOW_DATA *memWin,
	SHARED_QUEUE *queue
);

#endif
