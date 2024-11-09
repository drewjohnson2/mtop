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

void run_cpu_graph(
	Arena *graphArena,
	WINDOW_DATA *cpuWin,
	SHARED_QUEUE *q
)
{
	CPU_STATS *prevStats = NULL;
	CPU_STATS *curStats = NULL;

	prevStats = peek(q, &cpuQueueLock, &cpuQueueCondition);
	dequeue(q, &cpuQueueLock, &cpuQueueCondition);

	GRAPH_DATA *cpuGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	Arena cpuPointArena = a_new(sizeof(GRAPH_POINT));

	float cpuPercentage;
	int cont = 1;

	while (cont)
	{
		// I tried moving this to an IO thread.
		// Since we grab CPU/Mem info so often
		// it slowed execution to an insane degree.
		// Perhaps a queue would be nice?
		curStats = peek(q, &cpuQueueLock, &cpuQueueCondition);
		dequeue(q, &cpuQueueLock, &cpuQueueCondition);
 
		CALCULATE_CPU_PERCENTAGE(prevStats, curStats, cpuPercentage);

		add_graph_point(&cpuPointArena, cpuGraphData, cpuPercentage);
		graph_render(&cpuPointArena, cpuGraphData, cpuWin);

		prevStats = curStats;

		REFRESH_WIN(cpuWin->window);
		SHOULD_MERGE(&runLock, cont);
	}

	a_free(&cpuPointArena);
}

void run_memory_graph(
	Arena *graphArena,
	WINDOW_DATA *memWin,
	SHARED_QUEUE *queue	
)
{
	GRAPH_DATA *memGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	Arena memPointArena = a_new(sizeof(GRAPH_POINT));

	float memoryPercentage;
	int cont = 1;

	while (cont)
	{
		// I tried moving this to an IO thread.
		// Since we grab CPU/Mem info so often
		// it slowed execution to an insane degree.
		// Perhaps a queue would be nice?
		MEMORY_STATS *memStats = peek(queue, &memQueueLock, &memQueueCondition);
		dequeue(queue, &memQueueLock, &memQueueCondition);
 
		CALCULATE_MEMORY_USAGE(memStats, memoryPercentage);

		add_graph_point(&memPointArena, memGraphData, memoryPercentage);
		graph_render(&memPointArena, memGraphData, memWin);

		REFRESH_WIN(memWin->window);
		SHOULD_MERGE(&runLock, cont);
	}

	a_free(&memPointArena);
}
