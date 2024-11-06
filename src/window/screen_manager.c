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
	shared_data **sd
)
{
	GRAPH_DATA *cpuGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	GRAPH_DATA *memGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	Arena cpuPointArena = a_new(sizeof(GRAPH_POINT));
	Arena memPointArena = a_new(sizeof(GRAPH_POINT));

	float cpuPercentage, memoryPercentage;
	int cont = 1;

	while (cont)
	{
		pthread_mutex_lock(statsLock);

		while ((*sd)->readingFile) pthread_cond_wait(fileCond, statsLock);

		cpuPercentage = calculate_cpu_usage((*sd)->prev, (*sd)->cur);
		memoryPercentage = calculate_ram_usage((*sd)->memStats);

		(*sd)->prev = (*sd)->cur;

		napms(350);
		(*sd)->readingFile = 1;
		pthread_mutex_unlock(statsLock);

		add_graph_point(&cpuPointArena, cpuGraphData, cpuPercentage);
		add_graph_point(&memPointArena, memGraphData, memoryPercentage);

		graph_render(&cpuPointArena, cpuGraphData, cpuWin);
		graph_render(&memPointArena, memGraphData, memWin);

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
	shared_data **sd
)
{
	int cont = 1;

	while (cont)
	{
		pthread_mutex_lock(statsLock);

		(*sd)->cur = fetch_cpu_stats(cpuArena);
		(*sd)->memStats = fetch_ram_stats(memArena);

		(*sd)->readingFile = 0;

		pthread_mutex_unlock(statsLock);
		pthread_cond_signal(fileCond);

		SHOULD_MERGE(breakMutex, cont);
	}
}

// 80895745
