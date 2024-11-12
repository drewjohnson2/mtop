#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <unistd.h>
#include <arena.h>

#include "../include/thread/thread.h"
#include "../include/graph.h"
#include "../include/thread/ui_thread.h"
#include "../include/monitor/cpu_monitor.h"
#include "../include/util/shared_queue.h"
#include "../include/monitor/mem_monitor.h"

void run_graphs(
	Arena *graphArena,
	Arena *memGraphArena,
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
	WINDOW_DATA *container = di->windows[CONTAINER_WIN];

	prevStats = peek(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
	dequeue(cpuQueue, &cpuQueueLock, &cpuQueueCondition);

	GRAPH_DATA *cpuGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), __alignof(GRAPH_DATA));
	Arena cpuPointArena = a_new(sizeof(GRAPH_POINT));

	GRAPH_DATA *memGraphData = a_alloc(memGraphArena, sizeof(GRAPH_DATA), __alignof(GRAPH_DATA));
	Arena memPointArena = a_new(sizeof(GRAPH_POINT));

	float cpuPercentage, memoryPercentage;
	int cont = 1;

	while (cont)
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

		REFRESH_WIN(container->window);

		usleep(1000 * 200);

		SHOULD_MERGE(&runLock, cont);
	}

	a_free(&cpuPointArena);
	a_free(&memPointArena);
}
