#include <bits/time.h>
#include <pthread.h>
#include <unistd.h>
#include <arena.h>
#include <time.h>

#include "../include/thread.h"
#include "../include/monitor.h"
#include "../include/thread_safe_queue.h"
#include "../include/thread.h"
#include "../include/sorting.h"

void run_io(
    Arena *cpuArena,
    Arena *memArena,
    Arena *procArena,
    ThreadSafeQueue *cpuQueue,
    ThreadSafeQueue *memQueue,
    ThreadSafeQueue *procQueue
) 
{
    pthread_mutex_lock(&procDataLock);
    get_processes(procArena, prc_pid_compare);  
    pthread_mutex_unlock(&procDataLock);
    
    struct timespec start, current;
    
    clock_gettime(CLOCK_REALTIME, &start);
    
    enqueue(
    	procQueue,
    	get_processes(procArena, prc_pid_compare),
    	&procDataLock,
    	&procQueueCondition
    );
    
    while (!SHUTDOWN_FLAG)
    {
    	// This check prevents lag between the read and display of stats
    	// without it the points on the graph can be several seconds behind.
    	u8 minimumMet = cpuQueue->size < MIN_QUEUE_SIZE || memQueue->size < MIN_QUEUE_SIZE;
    
    	if (minimumMet) 
    	{
	    // I think this head free is causing an intermittent segfault. 
	    // only happned once though.
	    // UPDATE
	    // This free is fine as far as I can tell. Maybe if I run it long enough it'll fail.
	    // But adding this same check and freeing the head of the memory arena causes a crash 
	    // usually in five minutes. I need a way to clean up that arena.
	    // TODO: clean up the memory arena
	    // NOTE: this could be tough
	    if (cpuArena->regionsAllocated > MIN_QUEUE_SIZE && cpuArena->regionsAllocated > 1) r_free_head(cpuArena);

	    CpuStats *cpuStats = fetch_cpu_stats(cpuArena);
	    MemoryStats *memStats = fetch_memory_stats(memArena);
	    
	    enqueue(cpuQueue, cpuStats, &cpuQueueLock, &cpuQueueCondition);
	    enqueue(memQueue, memStats, &memQueueLock, &memQueueCondition);
    	}
    
    	clock_gettime(CLOCK_REALTIME, &current);
    
    	s32 totalTimeSec = current.tv_sec - start.tv_sec;
    
    	if (totalTimeSec > PROC_WAIT_TIME_SEC)
    	{
	    ProcessStats *stats = get_processes(procArena, prc_pid_compare);
	    
	    enqueue(
		procQueue,
		stats,
		&procDataLock,
		&procQueueCondition
	    );
    
	    clock_gettime(CLOCK_REALTIME, &start);
    	}
    
	usleep(READ_SLEEP_TIME);
    }
}
