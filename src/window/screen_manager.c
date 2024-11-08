#include <pthread.h>
#include <arena.h>
#include <ncurses.h>
#include <unistd.h>

#include "../include/ram_monitor.h"
#include "../include/cpu_monitor.h"
#include "../include/graph.h"
#include "../include/screen_manager.h"

void run_screen(
	Arena *cpuArena,
	Arena *memArena,
	Arena *graphArena,
	WINDOW_DATA *cpuWin,
	WINDOW_DATA *memWin,
	pthread_mutex_t *mutex,
	pthread_mutex_t *statsLock,
	pthread_cond_t *fileCond,
	shared_data **sd,
	QUEUE *q
)
{
	CPU_STATS *prev = NULL;
	CPU_STATS *cur = NULL;

	prev = peek(q, statsLock, fileCond);
	dequeue(q, statsLock, fileCond);

	GRAPH_DATA *cpuGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	GRAPH_DATA *memGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	Arena cpuPointArena = a_new(sizeof(GRAPH_POINT));
	Arena memPointArena = a_new(sizeof(GRAPH_POINT));
	float cpuPercentage, memoryPercentage;
	int cont = 1;
	
	while (cont)
	{
		cur = peek(q, statsLock, fileCond);
		dequeue(q, statsLock, fileCond);

		if (cur == NULL || prev == NULL) continue;

		cpuPercentage = calculate_cpu_usage(prev, cur);
		//memoryPercentage = calculate_ram_usage((*sd)->memStats);

		prev = cur;

		napms(200);

		add_graph_point(&cpuPointArena, cpuGraphData, cpuPercentage);
		//add_graph_point(&memPointArena, memGraphData, memoryPercentage);

		graph_render(&cpuPointArena, cpuGraphData, cpuWin);
		//graph_render(&memPointArena, memGraphData, memWin);

		touchwin(cpuWin->window);
		touchwin(memWin->window);
		wrefresh(cpuWin->window);
		wrefresh(memWin->window);

		SHOULD_MERGE(mutex, cont);
	}

	a_free(&cpuPointArena);
	a_free(&memPointArena);
}

void run_io(
	Arena *cpuArena,
	Arena *memArena,
	pthread_mutex_t *breakMutex,
	pthread_mutex_t *statsLock,
	pthread_cond_t *fileCond,
	shared_data **sd,
	QUEUE *q
)
{
	int cont = 1;

	while (cont)
	{
		//(*sd)->cur = fetch_cpu_stats(cpuArena);
		//(*sd)->memStats = fetch_ram_stats(memArena);

		enqueue(q, fetch_cpu_stats(cpuArena), statsLock, fileCond);

		SHOULD_MERGE(breakMutex, cont);
	}
}

// 80895745
