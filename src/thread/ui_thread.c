#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <unistd.h>
#include <arena.h>

#include "../include/thread/thread.h"
#include "../include/graph.h"
#include "../include/thread/ui_thread.h"
#include "../include/cpu_monitor.h"
#include "../include/util/shared_queue.h"
#include "../include/mem_monitor.h"

void run_graphs(
	Arena *graphArena,
	Arena *memGraphArena,
	WINDOW_DATA *cpuWin,
	WINDOW_DATA *memWin,
	SHARED_QUEUE *cpuQueue,
	SHARED_QUEUE *memoryQueue	
)
{
	CPU_STATS *prevStats = NULL;
	CPU_STATS *curStats = NULL;

	prevStats = peek(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
	dequeue(cpuQueue, &cpuQueueLock, &cpuQueueCondition);

	GRAPH_DATA *cpuGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	Arena cpuPointArena = a_new(sizeof(GRAPH_POINT));

	GRAPH_DATA *memGraphData = a_alloc(memGraphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	Arena memPointArena = a_new(sizeof(GRAPH_POINT));

	float cpuPercentage, memoryPercentage;
	int cont = 1;

	while (cont)
	{
		// I tried moving this to an IO thread.
		// Since we grab CPU/Mem info so often
		// it slowed execution to an insane degree.
		// Perhaps a queue would be nice?
		curStats = peek(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
		dequeue(cpuQueue, &cpuQueueLock, &cpuQueueCondition);

 		MEMORY_STATS *memStats = peek(memoryQueue, &memQueueLock, &memQueueCondition);
		dequeue(memoryQueue, &memQueueLock, &memQueueCondition);
 
		CALCULATE_MEMORY_USAGE(memStats, memoryPercentage);
		CALCULATE_CPU_PERCENTAGE(prevStats, curStats, cpuPercentage);

		add_graph_point(&cpuPointArena, cpuGraphData, cpuPercentage);
		graph_render(&cpuPointArena, cpuGraphData, cpuWin);

		add_graph_point(&memPointArena, memGraphData, memoryPercentage);
		graph_render(&memPointArena, memGraphData, memWin);

		prevStats = curStats;

		REFRESH_WIN(cpuWin->window);
		REFRESH_WIN(memWin->window);

		usleep(1000 * 200);

		SHOULD_MERGE(&runLock, cont);
	}

	a_free(&cpuPointArena);
	a_free(&memPointArena);
}
