#if defined (__linux__)
#include <bits/time.h>
#endif

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <arena.h>
#include <time.h>

#include "../../include/thread.h"
#include "../../include/monitor.h"
#include "../../include/sorting.h"
#include "../../include/prc_list_util.h"
#include "../../include/task.h"

static void _fetch_prcs(
	Arena *procArena,
	struct timespec *start,
	struct timespec *current,
	ProcessListState *listState,
	u8 prcActive
);
static void _fetch_prc_info(ProcessListState *listState);
static void _copy_stats(CpuStats *prevStats, CpuStats *curStats);

ProcessesSummary *prevPrcs;
ProcessesSummary *curPrcs;

void run_io(
    mtopArenas *arenas,
    WindowData **windows
) 
{
    Arena *cpuArena = arenas->cpuArena;
    Arena *procArena = arenas->prcArena;
    Arena *memArena = arenas->memArena;
    Arena *stateArena = arenas->stateArena;
    Arena tgArena = a_new(sizeof(TaskGroup));
    CpuStats *prevStats = a_alloc(cpuArena, sizeof(CpuStats), __alignof(CpuStats));
    CpuStats *curStats = a_alloc(cpuArena, sizeof(CpuStats), __alignof(CpuStats));
    ProcessListState *listState = a_alloc(
    	stateArena,
    	sizeof(ProcessListState),
    	__alignof(ProcessListState)
    );
    MemoryStats *memStats = NULL;
    struct timespec start, current;

    prevPrcs = pm_get_processes(procArena, prc_pid_compare);
    curPrcs = prevPrcs;
    memStats = a_alloc(memArena, sizeof(MemoryStats), __alignof(MemoryStats));

    plu_setup_list_state(listState, curPrcs, windows[PRC_WIN]);
    cm_fetch_cpu_stats(prevStats);
    clock_gettime(CLOCK_REALTIME, &start);
    init_data(arenas->cpuGraphArena, arenas->memoryGraphArena); 

    TaskGroup *tg = broker_create_group();

	BROKER_BUILD_TASK(tg, true, build_print_header_task, &tg->a);
	BROKER_BUILD_TASK(tg, true, build_print_footer_task, &tg->a);

	broker_commit(&tg);

    while (!SHUTDOWN_FLAG)
    {
		tg = broker_create_group();

		u8 cpuActive = mtopSettings->activeWindows[CPU_WIN];
    	u8 memActive = mtopSettings->activeWindows[MEMORY_WIN];
    	u8 prcActive = mtopSettings->activeWindows[PRC_WIN];

		if (cpuActive) cm_fetch_cpu_stats(curStats);
		if (memActive) mm_fetch_memory_stats(memStats);

		BROKER_BUILD_TASK(tg, true, build_input_task, &tg->a, listState);
		// BROKER_BUILD_TASK(tg, true, build_uptime_load_average_task, &tg->a);
		BROKER_BUILD_TASK(tg, true, build_print_time_task, &tg->a);

        if (prcActive)
        {
		    _fetch_prcs(procArena, &start, &current, listState, prcActive);
		    _fetch_prc_info(listState); // see comment above func
        }

		BROKER_BUILD_TASK(tg, cpuActive, build_cpu_task, &tg->a, arenas->cpuPointArena, curStats, prevStats);
		BROKER_BUILD_TASK(tg, memActive, build_mem_task, &tg->a, arenas->memPointArena, memStats);
		BROKER_BUILD_TASK(
			tg,
			prcActive,
			build_prc_task,
			&tg->a,
			listState,
			prevPrcs,
			curPrcs,
			memStats->memTotal
		);
		BROKER_BUILD_TASK(tg, RESIZE, build_resize_task, &tg->a, listState, curPrcs);
		BROKER_BUILD_TASK(tg, true, build_refresh_task, &tg->a);

		broker_commit(&tg);
		_copy_stats(prevStats, curStats);
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

static void _fetch_prcs(
	Arena *procArena,
	struct timespec *start,
	struct timespec *current,
	ProcessListState *listState,
	u8 prcActive
)
{
	clock_gettime(CLOCK_REALTIME, current);

	const s32 totalTimeSec = current->tv_sec - start->tv_sec;
	
	if ((totalTimeSec > PROC_WAIT_TIME_SEC) && prcActive)
	{
		prevPrcs = curPrcs;
		curPrcs = pm_get_processes(procArena, prc_pid_compare);
	
		plu_adjust_state(listState, curPrcs);
		
		clock_gettime(CLOCK_REALTIME, start);
	}
}

// I really like the stats to update quickly when 
// on the process info page, so that's why this is here.
static void _fetch_prc_info(ProcessListState *listState)
{
	if (listState->infoVisible)
	{
	    qsort(
			curPrcs->processes,
			curPrcs->count,
			sizeof(ProcessesSummary *),
			prc_pid_compare_without_direction_fn
	    );

	    Process **prc = bsearch(
			&listState->selectedPid,
			curPrcs->processes,
			curPrcs->count,
			sizeof(Process *),
			prc_find_by_pid_compare_fn
	    );

	    if (prc) pm_get_info_by_pid(listState->selectedPid, *prc);
	}
}
