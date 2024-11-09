#ifndef IO_THREAD_H
#define IO_THREAD_H

#include <pthread.h>
#include <arena.h>

#include "../util/shared_queue.h"

void run_io(
	Arena *cpuArena,
	Arena *memArena,
	SHARED_QUEUE *cpuQueue,
	SHARED_QUEUE *memQueue
);

#endif
