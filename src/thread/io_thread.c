#include <bits/time.h>
#include <pthread.h>
#include <unistd.h>
#include <arena.h>
#include <time.h>

#include "../include/thread.h"
#include "../include/monitor.h"
#include "../include/thread_safe_queue.h"
#include "../include/thread.h"
#include "../include/ui_utils.h"

void run_io(
	Arena *cpuArena,
	Arena *memArena,
	Arena *procArena,
	ThreadSafeQueue *cpuQueue,
	ThreadSafeQueue *memQueue
) 
{
	pthread_mutex_lock(&procDataLock);
	get_processes(procArena, proc_name_compare);  
	pthread_mutex_unlock(&procDataLock);

	struct timespec start, current;

	clock_gettime(CLOCK_REALTIME, &start);

	while (!SHUTDOWN_FLAG)
	{
		// This check prevents lag between the read and display of stats
		// without it the points on the graph can be several seconds behind.
		int minimumMet = cpuQueue->size < MIN_QUEUE_SIZE || memQueue->size < MIN_QUEUE_SIZE;

		if (minimumMet) 
		{
			CpuStats *cpuStats = fetch_cpu_stats(cpuArena);
			MemoryStats *memStats = fetch_memory_stats(memArena);

			enqueue(cpuQueue, cpuStats, &cpuQueueLock, &cpuQueueCondition);
			enqueue(memQueue, memStats, &memQueueLock, &memQueueCondition);
		}

		clock_gettime(CLOCK_REALTIME, &current);

		int totalTimeSec = current.tv_sec - start.tv_sec;

		if (totalTimeSec > PROC_WAIT_TIME)
		{
			pthread_mutex_lock(&procDataLock);
			get_processes(procArena, proc_name_compare);  
			pthread_mutex_unlock(&procDataLock);
			clock_gettime(CLOCK_REALTIME, &start);
		}

		usleep(READ_SLEEP_TIME);
	}
}
