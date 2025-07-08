#include <bits/time.h>
#include <pthread.h>
#include <unistd.h>
#include <arena.h>
#include <time.h>

#include "../../include/thread.h"
#include "../../include/monitor.h"
#include "../../include/thread_safe_queue.h"
#include "../../include/thread.h"
#include "../../include/sorting.h"
#include "../../include/task.h"

Arena scratch;

static void _task_group_cleanup();

void run_io(
    mtopArenas *arenas,
    ThreadSafeQueue *cpuQueue,
    ThreadSafeQueue *procQueue,
    volatile MemoryStats *memStats,
    volatile ProcessInfoSharedData *prcInfoSd
) 
{
    Arena *cpuArena = arenas->cpuArena;
    Arena *procArena = arenas->prcArena;
    Arena tgArena = a_new(sizeof(TaskGroup));
    CpuStats *prevStats = fetch_cpu_stats(cpuArena);
    CpuStats *curStats = NULL;
    TaskGroup *tg = a_alloc(&tgArena, sizeof(TaskGroup), __alignof(TaskGroup));

    tg->tasksComplete = 1;
    tg->cleanup = _task_group_cleanup;

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

    init_data(arenas->cpuGraphArena); 

    // Create cleanup function on task group to free this arena
    scratch = a_new(sizeof(UITask));
    
    while (!SHUTDOWN_FLAG)
    {
	if (!tg->tasksComplete)
	{
	    usleep(READ_SLEEP_TIME);
	    continue;
	} 

	curStats = fetch_cpu_stats(cpuArena);
	UITask *task = build_cpu_task(&scratch, curStats, prevStats);

	tg->tasks = task;
	tg->tasksComplete = 0;

	enqueue(cpuQueue, tg, &cpuQueueLock, &cpuQueueCondition);

	task = tg->tasks->next;

	prevStats = curStats;
	
	pthread_mutex_lock(&memQueueLock);
	
	MEM_UPDATING = 1;
	
	fetch_memory_stats(memStats);
	
	MEM_UPDATING = 0;
	
	pthread_cond_signal(&memQueueCondition);
	pthread_mutex_unlock(&memQueueLock);

    	clock_gettime(CLOCK_REALTIME, &current);
    
    	const s32 totalTimeSec = current.tv_sec - start.tv_sec;
    
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

	if (prcInfoSd->needsFetch && prcInfoSd->pidToFetch > 0)
	{
	    pthread_mutex_lock(&procInfoLock);
	    get_prc_info_by_pid(prcInfoSd); 

	    prcInfoSd->needsFetch = 0;

	    pthread_cond_signal(&procInfoCondition);
	    pthread_mutex_unlock(&procInfoLock);
	}
    
	usleep(READ_SLEEP_TIME);
    }
}

static void _task_group_cleanup()
{
    a_free(&scratch);

    scratch = a_new(sizeof(UITask));
}
