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
#include "../../include/prc_list_util.h"
#include "../../include/task.h"

#define BUILD_TASK(cond, data_fn, ...)	\
    do {					\
	if (cond)				\
	{					\
	    *tail = data_fn(__VA_ARGS__);	\
	    tail = &(*tail)->next;		\
	}					\
    } while(0)
	
ProcessesSummary *prevPrcs;
ProcessesSummary *curPrcs;

static void _copy_stats(CpuStats *prevStats, CpuStats *curStats);

void run_io(
    mtopArenas *arenas,
    ThreadSafeQueue *taskQueue,
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
    tg->a = a_new(64);
    tg->tasksComplete = 1;
    tg->cleanup = tg_cleanup;

    setup_list_state(listState, curPrcs, windows[PRC_WIN]);
    fetch_cpu_stats(prevStats);

    struct timespec start, current;
    
    clock_gettime(CLOCK_REALTIME, &start);
    
    init_data(arenas->cpuGraphArena, arenas->memoryGraphArena); 

    while (!SHUTDOWN_FLAG)
    {
	if (!tg->tasksComplete)
	{
	    usleep(READ_SLEEP_TIME);
	    continue;
	}

	UITask **tail = &tg->head;
	u8 handleCpu = cpuActive && tg->tasksComplete;
	u8 handleMem = memActive && tg->tasksComplete;

	if (handleCpu) fetch_cpu_stats(curStats);
	if (handleMem) fetch_memory_stats(memStats);

	BUILD_TASK(true, build_input_task, &tg->a, listState);
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

	BUILD_TASK(handleCpu, build_cpu_task, &tg->a, arenas->cpuPointArena, curStats, prevStats);
	BUILD_TASK(handleMem, build_mem_task, &tg->a, arenas->memPointArena, memStats);
	BUILD_TASK(prcActive, build_prc_task, &tg->a, listState, prevPrcs, curPrcs, processInfo,
	    memStats->memTotal);
	BUILD_TASK(RESIZE, build_resize_task, &tg->a, listState, curPrcs);
	BUILD_TASK(true, build_refresh_task, &tg->a);

	tg->tail = *tail;
	tg->tasksComplete = 0;

	_copy_stats(prevStats, curStats);

	enqueue(taskQueue, tg, &taskQueueLock, &taskQueueCondition);
	usleep(READ_SLEEP_TIME);
    }

    a_free(&tgArena);
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
