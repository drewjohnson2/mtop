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
static void _copy_stats(CpuStats *prevStats, CpuStats *curStats);

void run_io(
    mtopArenas *arenas,
    ThreadSafeQueue *cpuQueue,
    ThreadSafeQueue *procQueue,
    volatile ProcessInfoSharedData *prcInfoSd
) 
{
    Arena *cpuArena = arenas->cpuArena;
    Arena *procArena = arenas->prcArena;
    Arena *memArena = arenas->memArena;
    Arena tgArena = a_new(sizeof(TaskGroup));
    CpuStats *prevStats = a_alloc(cpuArena, sizeof(CpuStats), __alignof(CpuStats));
    CpuStats *curStats = a_alloc(cpuArena, sizeof(CpuStats), __alignof(CpuStats));

    fetch_cpu_stats(prevStats);

    MemoryStats *memStats = NULL;
    TaskGroup *tg = a_alloc(&tgArena, sizeof(TaskGroup), __alignof(TaskGroup));

    tg->tasksComplete = 1;
    tg->cleanup = _task_group_cleanup;

    memStats = a_alloc(memArena, sizeof(MemoryStats), __alignof(MemoryStats));

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

    init_data(arenas->cpuGraphArena, arenas->memoryGraphArena); 

    // Create cleanup function on task group to free this arena
    scratch = a_new(sizeof(UITask));
    
    while (!SHUTDOWN_FLAG)
    {
	if (!tg->tasksComplete)
	{
	    usleep(READ_SLEEP_TIME);
	    continue;
	} 

	fetch_cpu_stats(curStats);

	UITask *cpuTask = build_cpu_task(&scratch, arenas->cpuPointArena, curStats, prevStats);

	_copy_stats(prevStats, curStats);
	
	fetch_memory_stats(memStats);

	UITask *memTask = build_mem_task(&scratch, arenas->memPointArena, memStats);

	cpuTask->next = memTask;
	memTask->next = NULL;

	tg->tasks = cpuTask;
	tg->tasksComplete = 0;

	enqueue(cpuQueue, tg, &cpuQueueLock, &cpuQueueCondition);

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

    a_free(&tgArena);
}

static void _task_group_cleanup()
{
    a_free(&scratch);

    scratch = a_new(sizeof(UITask));
}

static void _copy_stats(CpuStats *prevStats, CpuStats *curStats)
{
    prevStats->cpuNumber = curStats->cpuNumber;
    prevStats->guest = curStats->guest;
    prevStats->guestNice = curStats->guestNice;
    prevStats->nice = curStats->nice;
    prevStats->idle = curStats->idle;
    prevStats->ioWait = curStats->ioWait;
    prevStats->irq = curStats->irq;
    prevStats->softIrq = curStats->softIrq;
    prevStats->steal = curStats->steal;
    prevStats->system = curStats->system;
    prevStats->user = curStats->user;
}
