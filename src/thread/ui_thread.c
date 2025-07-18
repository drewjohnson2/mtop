#include <bits/time.h>
#include <ncurses.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <arena.h>
#include <assert.h>

#include "../../include/thread.h"
#include "../../include/window.h"
#include "../../include/monitor.h"
#include "../../include/thread_safe_queue.h"
#include "../../include/mt_colors.h"
#include "../../include/sorting.h"
#include "../../include/startup.h"

#define STATE_A_SZ sizeof(ProcessListState) + __alignof(ProcessListState)
#define CPU_POINT_A_SZ sizeof(GraphPoint)
#define MEM_POINT_A_SZ sizeof(GraphPoint)

void run_ui(
    Arena *cpuGraphArena,
    Arena *memGraphArena,
    DisplayItems *di,
    ThreadSafeQueue *cpuQueue,
    ThreadSafeQueue *prcQueue,
    volatile MemoryStats *memStats,
    volatile ProcessInfoSharedData *prcInfoSd
)
{
    u64 memTotal = 0;
    float cpuPercentage, memoryPercentage;
    CpuStats *prevStats = NULL;
    CpuStats *curStats = NULL;
    const WindowData *cpuWin = di->windows[CPU_WIN];
    const WindowData *memWin = di->windows[MEMORY_WIN];
    const WindowData *prcWin = di->windows[PRC_WIN];
    const WindowData *optWin = di->windows[OPT_WIN];
    const WindowData *container = di->windows[CONTAINER_WIN];
    GraphData *cpuGraphData = a_alloc(cpuGraphArena, sizeof(GraphData), __alignof(GraphData));
    const u8 cpuActive = mtopSettings->activeWindows[CPU_WIN];
    const u8 memActive = mtopSettings->activeWindows[MEMORY_WIN];
    const u8 prcActive = mtopSettings->activeWindows[PRC_WIN];
    
    ProcessStats *prevPrcs = NULL;
    ProcessStats *curPrcs = NULL;
    
    GraphData *memGraphData = a_alloc(
        memGraphArena,
    	sizeof(GraphData), 
    	__alignof(GraphData)
    );
    
    Arena stateArena = a_new(STATE_A_SZ);
    Arena cpuPointArena = a_new(CPU_POINT_A_SZ);
    Arena memPointArena = a_new(MEM_POINT_A_SZ);
    
    // probably need to add some sort of shut down error
    // handling here.
    prevStats = peek(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
    dequeue(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
    
    prevPrcs = peek(prcQueue, &procDataLock, &procQueueCondition);
    dequeue(prcQueue, &procDataLock, &procQueueCondition);
    
    curPrcs = prevPrcs;
    
    ProcessListState *listState = a_alloc(
    	&stateArena,
    	sizeof(ProcessListState),
    	__alignof(ProcessListState)
    );
    
    listState->cmdBuffer = '\0';
    listState->timeoutActive = 0;
    listState->selectedIndex = 0;
    listState->pageStartIdx = 0;
    listState->count = curPrcs->count;
    listState->pageSize = prcWin->wHeight - 5;
    listState->totalPages = listState->count / listState->pageSize;

    if (listState->count % listState->pageSize > 0) listState->totalPages++;

    listState->pageEndIdx = listState->pageSize - 1;

    if (listState->pageEndIdx > listState->count)
	listState->pageEndIdx = listState->count - 1;

    listState->sortFunc = vd_name_compare_func;
    listState->sortOrder = PRC_NAME;
    listState->infoVisible = 0;

    import_colors();

    if (!mtopSettings->transparencyEnabled)
    {
	set_bg_colors(
	    container->window,
	    cpuWin->window,
	    memWin->window,
	    prcWin->window,
	    optWin->window
	);
    }

    print_header(container);
    print_footer(container);
    
    while (!SHUTDOWN_FLAG)
    {
	print_uptime_ldAvg(container);
	print_time(container);

    	curStats = peek(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
    	dequeue(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
    
	pthread_mutex_lock(&memQueueLock);

	while (MEM_UPDATING) 
	    pthread_cond_wait(&memQueueCondition, &memQueueLock);

    	CALCULATE_MEMORY_USAGE(memStats, memoryPercentage);

	memTotal = memStats->memTotal;

	pthread_mutex_unlock(&memQueueLock);

    	CALCULATE_CPU_PERCENTAGE(prevStats, curStats, cpuPercentage);
    
    	add_graph_point(&cpuPointArena, cpuGraphData, cpuPercentage, cpuActive);
    	graph_render(
	    &cpuPointArena,
	    cpuGraphData,
	    cpuWin,
	    MT_PAIR_CPU_GP,
	    MT_PAIR_CPU_HEADER,
	    cpuActive
	);
    
    	add_graph_point(&memPointArena, memGraphData, memoryPercentage, memActive);
    	graph_render(
	    &memPointArena,
	    memGraphData,
	    memWin,
	    MT_PAIR_MEM_GP,
	    MT_PAIR_MEM_HEADER,
	    memActive
	);
    
    	prevStats = curStats;
    
    	if (prcQueue->size > 0)
    	{
	    prevPrcs = curPrcs;
	    curPrcs = peek(prcQueue, &procDataLock, &procQueueCondition);
	    dequeue(prcQueue, &procDataLock, &procQueueCondition);
	    
	    adjust_state(listState, curPrcs);
    	}
    
    	Arena scratch = a_new(
	   (sizeof(ProcessStatsViewData *) * curPrcs->count) +
	   sizeof(ProcessStatsViewData) +
	   (sizeof(ProcessStatsViewData) * curPrcs->count)
    	);
    
    	ProcessStatsViewData **vd = a_alloc(
	    &scratch,
	    sizeof(ProcessStatsViewData *) * curPrcs->count,
	    __alignof(ProcessStatsViewData *)
    	); 
    	
    	set_prc_view_data(
	    &scratch,
	    vd,
	    curPrcs,
	    prevPrcs,
	    memTotal
    	);		

	if (prcActive)
	{
	    qsort(
	        vd,
	        curPrcs->count,
	        sizeof(ProcessStatsViewData *),
	        listState->sortFunc
	    );
	}

	read_input(container->window, listState, di, vd, prcInfoSd);

	if (!listState->infoVisible)
	{
	   print_stats(
	       listState,
	       prcWin,
	       vd,
	       curPrcs->count
    	   );
	}
	else 
	{
	    pthread_mutex_lock(&procInfoLock);

	    while (prcInfoSd->needsFetch)
		pthread_cond_wait(&procInfoCondition, &procInfoLock);

	    show_prc_info(vd[listState->selectedIndex], prcInfoSd->info, prcWin);
	    pthread_mutex_unlock(&procInfoLock);
	}
    
    	a_free(&scratch);
    
	// Normally I'd remove the else case and put the
	// REFRESH_WIN(container->window) above the if statement.
	// For whatever reason that setup causes bad flickering
	// on some machines. I guess on faster machines display_options
	// takes enough time between the calls to REFRESH_WIN for ncurses
	// to repaint the screen.
	if (di->optionsVisible)
	{
	    display_options(di);	    
	    REFRESH_WIN(container->window);
	    REFRESH_WIN(optWin->window);
	}
	else 
	{
	    REFRESH_WIN(container->window);
	}

	if (listState->infoVisible)
	{
	    prcInfoSd->needsFetch = 1;
	}
    
    	usleep(DISPLAY_SLEEP_TIME);
    }
    
    a_free(&cpuPointArena);
    a_free(&memPointArena);
    a_free(&stateArena);
}
