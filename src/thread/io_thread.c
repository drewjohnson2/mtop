#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <unistd.h>
#include <arena.h>

#include "../include/thread/thread.h"
#include "../include/monitor/mem_monitor.h"
#include "../include/monitor/cpu_monitor.h"
#include "../include/util/shared_queue.h"

void run_io(
	Arena *cpuArena,
	Arena *memArena,
	SHARED_QUEUE *cpuQueue,
	SHARED_QUEUE *memQueue
) 
{
	int cont = 1;

	while (cont)
	{
		// need to find a better way to synchronize the sleeps 
		// here and in graph.c
		usleep(1000 * 100);

		if (cpuQueue->size < 5 || memQueue->size < 5) 
		{
			CPU_STATS *cpuStats = fetch_cpu_stats(cpuArena);
			MEMORY_STATS *memStats = fetch_memory_stats(memArena);

			enqueue(cpuQueue, cpuStats, &cpuQueueLock, &cpuQueueCondition);
			enqueue(memQueue, memStats, &memQueueLock, &memQueueCondition);
		}
		
		SHOULD_MERGE(&runLock, cont);
	}
}
