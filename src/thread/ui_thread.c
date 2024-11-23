#include <bits/time.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arena.h>

#include "../include/thread.h"
#include "../include/window.h"
#include "../include/monitor.h"
#include "../include/thread_safe_queue.h"
#include "../include/startup.h"
#include "../include/ui_utils.h"

typedef struct _stats_view_data 
{
	char *command;
	int pid;
	float cpuPercentage;
	float memPercentage;
} StatsViewData;

ProcessStats * _create_stats_copy(Arena *arena);
void _print_stats(WindowData *wd, StatsViewData vd[], int count, Arena *procArena);

void run_ui(
	Arena *graphArena,
	Arena *memGraphArena,
	Arena *procArena,
	DisplayItems *di,
	ThreadSafeQueue *cpuQueue,
	ThreadSafeQueue *memoryQueue,
	ThreadSafeQueue *procQueue
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

	Arena cpuPointArena = a_new(sizeof(GraphPoint));
	Arena memPointArena = a_new(sizeof(GraphPoint));
	Arena procUiArena = a_new(1024);

	ProcessStats *prevProcSample = NULL;
	ProcessStats *curProcSample = NULL;

	GraphData *memGraphData = a_alloc(
		memGraphArena,
		sizeof(GraphData), 
		__alignof(GraphData)
	);

	// probably need to add some sort of shut down error
	// handling here.
	prevStats = peek(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
	dequeue(cpuQueue, &cpuQueueLock, &cpuQueueCondition);

	prevProcSample = peek(procQueue, &procDataLock, &procQueueCondition);
	dequeue(procQueue, &procDataLock, &procQueueCondition);
	
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

		if (procQueue->size > 0)
		{
			curProcSample = peek(procQueue, &procDataLock, &procQueueCondition);
			dequeue(procQueue, &procDataLock, &procQueueCondition);
		}

		ProcessStats *procs = curProcSample == NULL ?
			prevProcSample :
			curProcSample;

		StatsViewData vd[procs->count]; 

		for (int i = 0; i < procs->count; i++)
		{
			vd[i] = (StatsViewData){
				.command = procs->processes[i]->procName,
				.pid = procs->processes[i]->pid,
				.cpuPercentage = 0,
				.memPercentage = 0
	   		};
		}

		// There was once a two second 
		// timer check here, if things
		// get wonky put it back
		_print_stats(procWin, vd, procs->count, procArena);

		if (curProcSample != NULL)
			prevProcSample = curProcSample;

		REFRESH_WIN(container->window);

		usleep(DISPLAY_SLEEP_TIME);
	}

	a_free(&cpuPointArena);
	a_free(&memPointArena);
	a_free(&procUiArena);
}

// ProcessStats * _create_stats_copy(Arena *arena)
// {
// 	pthread_mutex_lock(&procDataLock);
//
// 	ProcessStats *stat = a_alloc(arena, sizeof(ProcessStats), __alignof(ProcessStats));
//
// 	stat->count = procStats->count;
// 	stat->cpuTimeAtSample = procStats->cpuTimeAtSample;
//
// 	stat->processes = a_alloc(
// 		arena,
// 		sizeof(ProcessList *) * procStats->count,
// 		__alignof(ProcessList *)
// 	);
//
// 	for (int i = 0; i < procStats->count; i++)
// 	{
// 		char * command = a_strdup(arena, procStats->processes[i]->procName);
//
// 		stat->processes[i] = a_alloc(arena, sizeof(ProcessList), __alignof(ProcessList));
// 		stat->processes[i]->pid = procStats->processes[i]->pid;
// 		stat->processes[i]->stime = procStats->processes[i]->stime;
// 		stat->processes[i]->utime = procStats->processes[i]->utime;
// 		strcpy(stat->processes[i]->procName, command);	
// 	}
//
// 	pthread_mutex_unlock(&procDataLock);
//
// 	return stat;
// }

void _print_stats(WindowData *wd, StatsViewData vd[], int count, Arena *procArena)
{
	if (vd == NULL) return;

	char *commandTitle = "Command";
	char *pidTitle = "PID";
	char *cpuTitle = "CPU %";
	char *memTitle = "Memory %";
	u16 pidPosX = wd->wWidth * .60;
	u16 cpuPosX = pidPosX + (wd->wWidth * .14);
	u16 memPosX = cpuPosX + (wd->wWidth * .14);

	int fitMemory = wd->wWidth >= memPosX + strlen(memTitle);

	if (!fitMemory) 
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
	if (fitMemory) mvwprintw(win, 2, memPosX, "%s", memTitle);

	int x = 2;

	while (x < wd->wWidth - 3)
	{
		mvwprintw(win, 3, x++, "%c", '-');
	}

	wattroff(win, A_BOLD);

	int i = 0;
	int posY = 4;

	//pthread_mutex_lock(&procDataLock);

	while (i < wd->wHeight - 5 && i < count)
	{
		mvwprintw(win, posY, 2, "%s", vd[i].command);
		mvwprintw(win, posY, pidPosX, "%d", vd[i].pid);

		if (fitCpu) mvwprintw(win, posY, cpuPosX, "%.2f", vd[i].cpuPercentage);
		if (fitMemory) mvwprintw(win, posY++, memPosX, "%.2f", (float)rand()/(float)(RAND_MAX/2));

		i++;
	}

	//pthread_mutex_unlock(&procDataLock);
}
