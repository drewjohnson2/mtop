#include <bits/time.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arena.h>

#include "../include/thread/thread.h"
#include "../include/graph.h"
#include "../include/thread/ui_thread.h"
#include "../include/monitor/cpu_monitor.h"
#include "../include/util/shared_queue.h"
#include "../include/monitor/mem_monitor.h"
#include "../include/monitor/proc_monitor.h"
#include "../include/startup/startup.h"

void print_stats(WINDOW_DATA *wd, Arena *procArena);

void run_ui(
	Arena *graphArena,
	Arena *memGraphArena,
	Arena *procArena,
	DISPLAY_ITEMS *di,
	SHARED_QUEUE *cpuQueue,
	SHARED_QUEUE *memoryQueue
)
{
	CPU_STATS *prevStats = NULL;
	CPU_STATS *curStats = NULL;
	MEMORY_STATS *memStats = NULL;
	WINDOW_DATA *cpuWin = di->windows[CPU_WIN];
	WINDOW_DATA *memWin = di->windows[MEMORY_WIN];
	WINDOW_DATA *procWin = di->windows[PRC_WIN];
	WINDOW_DATA *container = di->windows[CONTAINER_WIN];

	// probably need to add some sort of shut down error
	// handling here.
	prevStats = peek(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
	dequeue(cpuQueue, &cpuQueueLock, &cpuQueueCondition);

	GRAPH_DATA *cpuGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), __alignof(GRAPH_DATA));
	Arena cpuPointArena = a_new(sizeof(GRAPH_POINT));

	GRAPH_DATA *memGraphData = a_alloc(
		memGraphArena,
		sizeof(GRAPH_DATA), 
		__alignof(GRAPH_DATA)
	);

	float cpuPercentage, memoryPercentage;
	Arena memPointArena = a_new(sizeof(GRAPH_POINT));
	
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

		// There was once a two second 
		// timer check here, if things
		// get wonky put it back
		print_stats(procWin, procArena);

		REFRESH_WIN(container->window);

		usleep(DISPLAY_SLEEP_TIME);
	}

	a_free(&cpuPointArena);
	a_free(&memPointArena);
}

void print_stats(WINDOW_DATA *wd, Arena *procArena)
{
	if (procStats == NULL) return;

	char *commandTitle = "Command";
	char *pidTitle = "PID";
	char *cpuTitle = "CPU %";
	char *memTitle = "Memory %";
	unsigned short pidPosX = wd->wWidth * .60;
	unsigned short cpuPosX = pidPosX + (wd->wWidth * .14);
	unsigned short memPosX = cpuPosX + (wd->wWidth * .14);

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

	pthread_mutex_lock(&procDataLock);

	while (i < wd->wHeight - 5 && procStats[i] != NULL)
	{
		mvwprintw(win, posY, 2, "%s", procStats[i]->procName);
		mvwprintw(win, posY, pidPosX, "%d", procStats[i]->pid);

		if (fitCpu) mvwprintw(win, posY, cpuPosX, "%.2f", (float)rand()/(float)(RAND_MAX/2));
		if (fitMemory) mvwprintw(win, posY++, memPosX, "%.2f", (float)rand()/(float)(RAND_MAX/2));

		i++;
	}

	pthread_mutex_unlock(&procDataLock);
}
