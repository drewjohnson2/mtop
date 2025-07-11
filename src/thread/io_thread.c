#include <bits/time.h>
#include <pthread.h>
#include <unistd.h>
#include <arena.h>
#include <time.h>
#include <stdlib.h>

#include "../../include/thread.h"
#include "../../include/monitor.h"
#include "../../include/thread_safe_queue.h"
#include "../../include/thread.h"
#include "../../include/sorting.h"
#include "../../include/prc_list_util.h"
#include "../../include/task.h"

#define BUILD_TASK(cond, data_func, ...)	\
    do {					\
	if (cond)				\
	{					\
	    *tail = data_func(__VA_ARGS__);	\
	    tail = &(*tail)->next;		\
	}					\
    } while(0)
	

Arena taskArena;
ProcessesSummary *prevPrcs;
ProcessesSummary *curPrcs;

static void _task_group_cleanup();
static void _copy_stats(CpuStats *prevStats, CpuStats *curStats);

void run_io(
    mtopArenas *arenas,
    ThreadSafeQueue *cpuQueue,
    WindowData **windows
) 
{
    u8 cpuActive = mtopSettings->activeWindows[CPU_WIN];
    u8 memActive = mtopSettings->activeWindows[MEMORY_WIN];
    u8 prcActive = mtopSettings->activeWindows[PRC_WIN];
    Arena *cpuArena = arenas->cpuArena;
    Arena *procArena = arenas->prcArena;
    Arena *memArena = arenas->memArena;
    Arena *stateArena = arenas->stateArena;
    Arena *general = arenas->general;
    Arena tgArena = a_new(sizeof(TaskGroup));
    CpuStats *prevStats = a_alloc(cpuArena, sizeof(CpuStats), __alignof(CpuStats));
    CpuStats *curStats = a_alloc(cpuArena, sizeof(CpuStats), __alignof(CpuStats));
    ProcessListState *listState = a_alloc(
    	stateArena,
    	sizeof(ProcessListState),
    	__alignof(ProcessListState)
    );
    ProcessInfoData *processInfo = (ProcessInfoData *)a_alloc(
	general,
	sizeof(ProcessInfoData),
	__alignof(ProcessInfoData)
    ); 
    MemoryStats *memStats = NULL;
    TaskGroup *tg = a_alloc(&tgArena, sizeof(TaskGroup), __alignof(TaskGroup));

    processInfo->info = a_alloc(general, sizeof(ProcessInfo), __alignof(ProcessInfo));
    prevPrcs = get_processes(procArena, prc_pid_compare);
    curPrcs = prevPrcs;

    memStats = a_alloc(memArena, sizeof(MemoryStats), __alignof(MemoryStats));
    tg->tasksComplete = 1;
    tg->cleanup = _task_group_cleanup;

    setup_list_state(listState, curPrcs, windows[PRC_WIN]);
    fetch_cpu_stats(prevStats);

    // What is the purpose of this?
    // This was at the top of the io loop
    // in the previous implementation, but
    // nothing uses the value returned by
    // get_processes???
    // pthread_mutex_lock(&procDataLock);
    // get_processes(procArena, prc_pid_compare);  
    // pthread_mutex_unlock(&procDataLock);
    
    struct timespec start, current;
    
    clock_gettime(CLOCK_REALTIME, &start);
    
    init_data(arenas->cpuGraphArena, arenas->memoryGraphArena); 

    // Create cleanup function on task group to free this arena
    taskArena = a_new(64);
    
    while (!SHUTDOWN_FLAG)
    {
	if (!tg->tasksComplete)
	{
	    usleep(READ_SLEEP_TIME);
	    continue;
	}

	if (cpuActive) fetch_cpu_stats(curStats);
	if (memActive) fetch_memory_stats(memStats);
	clock_gettime(CLOCK_REALTIME, &current);
    
    	const s32 totalTimeSec = current.tv_sec - start.tv_sec;
    
    	if ((totalTimeSec > PROC_WAIT_TIME_SEC) && prcActive)
    	{
	    prevPrcs = curPrcs;
	    curPrcs = get_processes(procArena, prc_pid_compare);

	    adjust_state(listState, curPrcs);
	    
	    clock_gettime(CLOCK_REALTIME, &start);
    	}

	if ((listState->infoVisible && listState->selectedPid > 0) && prcActive)
	{
	    processInfo->pidToFetch = listState->selectedPid;
	    get_prc_info_by_pid(processInfo); 
	}

	UITask **tail = &tg->head;

	BUILD_TASK(cpuActive, build_cpu_task, &taskArena, arenas->cpuPointArena, curStats, prevStats);
	BUILD_TASK(memActive, build_mem_task, &taskArena, arenas->memPointArena, memStats);
	BUILD_TASK(true, build_input_task, &taskArena, listState);
	BUILD_TASK(
	    prcActive,
	    build_prc_task,
	    &taskArena,
	    listState,
	    prevPrcs,
	    curPrcs,
	    processInfo,
	    memStats->memTotal
	);
		
	tg->tail = *tail;
	tg->tasksComplete = 0;

	_copy_stats(prevStats, curStats);

	enqueue(cpuQueue, tg, &cpuQueueLock, &cpuQueueCondition);
	usleep(READ_SLEEP_TIME);
    }

    a_free(&tgArena);
}

static void _task_group_cleanup()
{
    a_free(&taskArena);

    taskArena = a_new(64);
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
