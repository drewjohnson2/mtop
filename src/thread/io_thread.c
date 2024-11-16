#include <bits/time.h>
#include <pthread.h>
#include <unistd.h>
#include <arena.h>
#include <time.h>

#include "../include/thread/thread.h"
#include "../include/monitor/mem_monitor.h"
#include "../include/monitor/cpu_monitor.h"
#include "../include/util/shared_queue.h"
#include "../include/thread/io_thread.h"

void run_io(
	Arena *cpuArena,
	Arena *memArena,
	Arena *procArena,
	SHARED_QUEUE *cpuQueue,
	SHARED_QUEUE *memQueue,
	PROC_STATS **stats
) 
{
	struct timespec start, current;

	clock_gettime(CLOCK_REALTIME, &start);

	while (!SHUTDOWN_FLAG)
	{
		int minimumMet = cpuQueue->size < MIN_QUEUE_SIZE || memQueue->size < MIN_QUEUE_SIZE;

		if (minimumMet) 
		{
			CPU_STATS *cpuStats = fetch_cpu_stats(cpuArena);
			MEMORY_STATS *memStats = fetch_memory_stats(memArena);

			enqueue(cpuQueue, cpuStats, &cpuQueueLock, &cpuQueueCondition);
			enqueue(memQueue, memStats, &memQueueLock, &memQueueCondition);
		}

		clock_gettime(CLOCK_REALTIME, &current);

		int totalTimeSecs = current.tv_sec - start.tv_sec;

		if (totalTimeSecs > 2)
		{
			pthread_mutex_lock(&procDataLock);
			stats = get_processes(procArena);  
			clock_gettime(CLOCK_REALTIME, &start);
			pthread_mutex_unlock(&procDataLock);
		}

		usleep(READ_SLEEP_TIME);
	}
}
