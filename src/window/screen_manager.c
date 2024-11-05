#include <arena.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/ram_monitor.h"
#include "../include/cpu_monitor.h"
#include "../include/graph.h"
#include "../include/screen_manager.h"
#include "../include/startup/startup.h"

void run_screen(
	Arena *cpuArena,
	Arena *memArena,
	Arena *graphArena,
	WINDOW_DATA *cpuWin,
	WINDOW_DATA *memWin,
	pthread_mutex_t *mutex,
	pthread_mutex_t *statsLock,
	pthread_cond_t *renderCondition
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

		napms(450);

		cpuPercentage = calculate_cpu_usage(prevStats, curStats);
		memoryPercentage = calculate_ram_usage(memStats);

		pthread_mutex_lock(statsLock);
		pthread_cond_wait(renderCondition, statsLock);

		add_graph_point(&cpuPointArena, cpuGraphData, cpuPercentage);
		add_graph_point(&memPointArena, memGraphData, memoryPercentage);

		graph_render(&cpuPointArena, cpuGraphData, cpuWin);
		graph_render(&memPointArena, memGraphData, memWin);

		pthread_mutex_unlock(statsLock);

		touchwin(cpuWin->window);
		touchwin(memWin->window);
		wrefresh(cpuWin->window);
		wrefresh(memWin->window);

		prevStats = curStats;

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
	pthread_cond_t *renderCondition
)
{
	prevStats = fetch_cpu_stats(cpuArena);

	int cont = 1;

	while (cont)
	{
		pthread_mutex_lock(statsLock);

		curStats = fetch_cpu_stats(cpuArena);
		memStats = fetch_ram_stats(memArena);

		pthread_mutex_unlock(statsLock);
		pthread_cond_signal(renderCondition);

		SHOULD_MERGE(breakMutex, cont);
	}
}
