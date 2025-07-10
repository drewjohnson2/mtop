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

Arena taskArena;
ProcessStats *prevPrcs;
ProcessStats *curPrcs;

static void _task_group_cleanup();
static void _copy_stats(CpuStats *prevStats, CpuStats *curStats);

void run_io(
    mtopArenas *arenas,
    ThreadSafeQueue *cpuQueue,
    volatile ProcessInfoData *prcInfoSd,
    WindowData **windows
) 
{
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
    processInfo->info = a_alloc(general, sizeof(ProcessInfo), __alignof(ProcessInfo));

    prevPrcs = get_processes(procArena, prc_pid_compare);
    curPrcs = prevPrcs;

    setup_list_state(listState, curPrcs, windows[PRC_WIN]);
    fetch_cpu_stats(prevStats);

    MemoryStats *memStats = NULL;
    TaskGroup *tg = a_alloc(&tgArena, sizeof(TaskGroup), __alignof(TaskGroup));

    memStats = a_alloc(memArena, sizeof(MemoryStats), __alignof(MemoryStats));
    tg->tasksComplete = 1;
    tg->cleanup = _task_group_cleanup;

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
    
 //    enqueue(
	// procQueue,
	// get_processes(procArena, prc_pid_compare),
	// &procDataLock,
	// &procQueueCondition
 //    );
	//
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

	fetch_cpu_stats(curStats);
	fetch_memory_stats(memStats);
	clock_gettime(CLOCK_REALTIME, &current);
    
    	const s32 totalTimeSec = current.tv_sec - start.tv_sec;
    
    	if (totalTimeSec > PROC_WAIT_TIME_SEC)
    	{
	    prevPrcs = curPrcs;
	    curPrcs = get_processes(procArena, prc_pid_compare);

	    // I need some sort of way to clean up the view data stuff.
	    // I certainly need to find a way to clean up the arena,
	    // but I'd also like to find some way to only use the
	    // view data. Idk, view data stuff just feels really ugly.
	    //
	    // Quick idea: track selected PID in the list state? 
	    // That way we can perform all of our input actions,
	    // then we can move the creation and allocation of the view
	    // data back to the process action. Possible????

	    adjust_state(listState, curPrcs);
	    
	    clock_gettime(CLOCK_REALTIME, &start);
    	}

	if (listState->infoVisible && listState->selectedPid > 0)
	{
	    processInfo->pidToFetch = listState->selectedPid;
	    get_prc_info_by_pid(processInfo); 
	}

	UITask *cpuTask = build_cpu_task(&taskArena, arenas->cpuPointArena, curStats, prevStats);
	UITask *memTask = build_mem_task(&taskArena, arenas->memPointArena, memStats);
	UITask *prcTask = build_prc_task(
	    &taskArena,
	    listState,
	    prevPrcs,
	    curPrcs,
	    processInfo,
	    memStats->memTotal
	);
	UITask *inputTask = build_input_task(&taskArena, listState);

	// this task order is very good.
	// makes the kb input run much
	// faster than it ever has.
	// Also: keep the sort function
	// for vd in the action, much, much faster.
	cpuTask->next = memTask;
	memTask->next = inputTask;
	inputTask->next = prcTask;

	tg->tasks = cpuTask;
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
