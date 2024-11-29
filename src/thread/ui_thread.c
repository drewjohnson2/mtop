#include <bits/time.h>
#include <ncurses.h>
#include <pthread.h>
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
#include "../include/ui_utils.h"
#include "../include/mt_colors.h"

typedef struct _stats_view_data 
{
	char *command;
	u32 pid;
	float cpuPercentage;
	float memPercentage;
} StatsViewData;

ProcessStats * _create_stats_copy(Arena *arena);
static void _print_stats(WindowData *wd, StatsViewData **vd, int count, Arena *procArena);

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

	Arena colorArena = a_new(512);

	MT_UI_Theme *theme = import_colors(&colorArena);

	wbkgd(container->window, COLOR_PAIR(MT_PAIR_BACKGROUND));

	Arena cpuPointArena = a_new(sizeof(GraphPoint));
	Arena memPointArena = a_new(sizeof(GraphPoint));

	ProcessStats *prevPrcs = NULL;
	ProcessStats *curPrcs = NULL;

	GraphData *memGraphData = a_alloc(
		memGraphArena,
		sizeof(GraphData), 
		__alignof(GraphData)
	);

	// probably need to add some sort of shut down error
	// handling here.
	prevStats = peek(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
	dequeue(cpuQueue, &cpuQueueLock, &cpuQueueCondition);

	prevPrcs = peek(prcQueue, &procDataLock, &procQueueCondition);
	dequeue(prcQueue, &procDataLock, &procQueueCondition);

	curPrcs = prevPrcs;
	
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
		}

		Arena scratch = a_new(
			(sizeof(StatsViewData *) * curPrcs->count) +
			sizeof(StatsViewData) +
			(sizeof(StatsViewData) * curPrcs->count)
		);

		StatsViewData **vd = a_alloc(
			&scratch,
			sizeof(StatsViewData *) * curPrcs->count,
			__alignof(StatsViewData *)
		); 
		
		for (int i = 0; i < curPrcs->count; i++)
		{
			float cpuPct = 0.0;
			float memPct = 0.0;
			u64 memTotal = memStats->memTotal;

			ProcessList *target;
			ProcessList *cur = curPrcs->processes[i];
			ProcessList **match = bsearch(
				&cur,
				prevPrcs->processes,
				prevPrcs->count,
				sizeof(ProcessList *),
				prc_pid_compare	
			);

			target = !match ? cur : *match;

			CALC_PRC_CPU_USAGE_PCT(
				target,
				cur,
				cpuPct,
				prevPrcs->cpuTimeAtSample,
				curPrcs->cpuTimeAtSample
			);

			memPct = memTotal > 0 ? 
				(cur->vmRss / (float)memTotal) * 100 :
				0;

			vd[i] = a_alloc(
				&scratch,
				sizeof(StatsViewData),
				__alignof(StatsViewData)
			);

			vd[i]->pid = cur->pid;
			vd[i]->command = cur->procName;
			vd[i]->cpuPercentage = cpuPct;
			vd[i]->memPercentage = memPct;	
		}

		// There was once a two second 
		// timer check here, if things
		// get wonky put it back
		_print_stats(procWin, vd, curPrcs->count, prcArena);

		a_free(&scratch);

		REFRESH_WIN(container->window);

		usleep(DISPLAY_SLEEP_TIME);
	}

	a_free(&cpuPointArena);
	a_free(&memPointArena);
	a_free(&colorArena);
}

int vd_name_compare_func(const void *a, const void *b)
{
	assert(a && b);

	const StatsViewData *x = *(StatsViewData **)a;
	const StatsViewData *y = *(StatsViewData **)b;

	return strcmp(x->command, y->command);
}

static void _print_stats(WindowData *wd, StatsViewData **vd, int count, Arena *procArena)
{
	if (vd == NULL) return;

	qsort(vd, count, sizeof(StatsViewData *), vd_name_compare_func);

	char *commandTitle = "Command";
	char *pidTitle = "PID";
	char *cpuTitle = "CPU %";
	char *memTitle = "Memory %";
	u16 pidPosX = wd->wWidth * .60;
	u16 cpuPosX = pidPosX + (wd->wWidth * .14);
	u16 memPosX = cpuPosX + (wd->wWidth * .14);

	int fitMem = wd->wWidth >= memPosX + strlen(memTitle);

	if (!fitMem) 
	{
		pidPosX = wd->wWidth * .70;
		cpuPosX = pidPosX + (wd->wWidth * .17);
	}

	int fitCpu = wd->wWidth >= cpuPosX + strlen(cpuTitle);
	
	WINDOW *win = wd->window;

	wattron(win, COLOR_PAIR(2));

	werase(win);
	box(win, 0, 0);

#ifdef DEBUG
	mvwprintw(win, 0, 3, " Arena Regions Alloc'd = %zu ", procArena->regionsAllocated);
#else
	mvwprintw(win, 0, 3, " %s ", wd->windowTitle);
#endif

	wattron(win, A_BOLD);

	mvwprintw(win, 2, 2, "%s", commandTitle);
	mvwprintw(win, 2, pidPosX, "%s", pidTitle);

	if (fitCpu) mvwprintw(win, 2, cpuPosX, "%s", cpuTitle);
	if (fitMem) mvwprintw(win, 2, memPosX, "%s", memTitle);

	int x = 2;

	while (x < wd->wWidth - 3)
	{
		mvwprintw(win, 3, x++, "%c", '-');
	}

	wattroff(win, A_BOLD);

	int i = 0;
	int posY = 4;

	while (i < wd->wHeight - 5 && i < count)
	{
		mvwprintw(win, posY, 2, "%s", vd[i]->command);
		mvwprintw(win, posY, pidPosX, "%d", vd[i]->pid);

		if (fitCpu) mvwprintw(win, posY, cpuPosX, "%.2f", vd[i]->cpuPercentage);
		if (fitMem) mvwprintw(win, posY++, memPosX, "%.2f", vd[i]->memPercentage);

		i++;
	}
}
