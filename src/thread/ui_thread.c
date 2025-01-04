#include <bits/time.h>
#include <ncurses.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arena.h>
#include <assert.h>

#include "../include/thread.h"
#include "../include/window.h"
#include "../include/monitor.h"
#include "../include/thread_safe_queue.h"
#include "../include/mt_colors.h"

ProcessStats * _create_stats_copy(Arena *arena);

void run_ui(
	Arena *graphArena,
	Arena *memGraphArena,
	Arena *prcArena,
	DisplayItems *di,
	ThreadSafeQueue *cpuQueue,
	ThreadSafeQueue *memoryQueue,
	ThreadSafeQueue *prcQueue
)
{
	float cpuPercentage, memoryPercentage;
	CpuStats *prevStats = NULL;
	CpuStats *curStats = NULL;
	MemoryStats *memStats = NULL;
	WindowData *cpuWin = di->windows[CPU_WIN];
	WindowData *memWin = di->windows[MEMORY_WIN];
	WindowData *procWin = di->windows[PRC_WIN];
	WindowData *container = di->windows[CONTAINER_WIN];
	GraphData *cpuGraphData = a_alloc(graphArena, sizeof(GraphData), __alignof(GraphData));
	Arena stateArena = a_new(sizeof(ProcessListState) + __alignof(ProcessListState));
	
	ProcessStats *prevPrcs = NULL;
	ProcessStats *curPrcs = NULL;

	GraphData *memGraphData = a_alloc(
		memGraphArena,
		sizeof(GraphData), 
		__alignof(GraphData)
	);

	Arena cpuPointArena = a_new(sizeof(GraphPoint));
	Arena memPointArena = a_new(sizeof(GraphPoint));

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
	listState->firstIndexDisplayed = 0;
	listState->lastIndexDisplayed = listState->numOptsVisible;
	listState->maxIndex = curPrcs->count - 1;
	listState->numOptsVisible = procWin->wHeight - 5;
	listState->lastIndexDisplayed = listState->numOptsVisible - 1;

	import_colors();
	wbkgd(container->window, COLOR_PAIR(MT_PAIR_BACKGROUND));
	
	while (!SHUTDOWN_FLAG)
	{
		curStats = peek(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
		dequeue(cpuQueue, &cpuQueueLock, &cpuQueueCondition);

 		memStats = peek(memoryQueue, &memQueueLock, &memQueueCondition);
		dequeue(memoryQueue, &memQueueLock, &memQueueCondition);
 
		CALCULATE_MEMORY_USAGE(memStats, memoryPercentage);
		CALCULATE_CPU_PERCENTAGE(prevStats, curStats, cpuPercentage);

		add_graph_point(&cpuPointArena, cpuGraphData, cpuPercentage);
		graph_render(&cpuPointArena, cpuGraphData, cpuWin);

		add_graph_point(&memPointArena, memGraphData, memoryPercentage);
		graph_render(&memPointArena, memGraphData, memWin);

		prevStats = curStats;

		if (prcQueue->size > 0)
		{
			prevPrcs = curPrcs;
			curPrcs = peek(prcQueue, &procDataLock, &procQueueCondition);
			dequeue(prcQueue, &procDataLock, &procQueueCondition);

			listState->maxIndex = curPrcs->count - 1;
			listState->selectedIndex = 
				listState->selectedIndex > listState->maxIndex ?
					listState->maxIndex :
					listState->selectedIndex;
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

		read_input(container->window, listState);
		
		set_prc_view_data(
			&scratch,
			vd,
			curPrcs,
			prevPrcs,
			memStats->memTotal
		);		

		// There was once a two second 
		// timer check here, if things
		// get wonky put it back
		print_stats(
			listState,
			procWin,
			vd,
			curPrcs->count,
			prcArena
		);

		a_free(&scratch);

		REFRESH_WIN(container->window);

		usleep(DISPLAY_SLEEP_TIME);
	}

	a_free(&cpuPointArena);
	a_free(&memPointArena);
	a_free(&stateArena);
}
