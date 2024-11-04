#include <arena.h>
#include <errno.h>
#include <pthread.h>

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
	pthread_mutex_t *ncursesLock
)
{
	CPU_STATS *prevStats = fetch_cpu_stats(cpuArena);
	GRAPH_DATA *cpuGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	GRAPH_DATA *memGraphData = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	Arena cpuPointArena = a_new(sizeof(GRAPH_POINT));
	Arena memPointArena = a_new(sizeof(GRAPH_POINT));

	float cpuPercentage, memoryPercentage;
	int cont = 1;

	while (cont)
	{
		napms(350);

		CPU_STATS *curStats = fetch_cpu_stats(cpuArena);
		RAM_STATS *memStats = fetch_ram_stats(memArena);

		cpuPercentage = calculate_cpu_usage(prevStats, curStats);
		memoryPercentage = calculate_ram_usage(memStats);

		add_graph_point(&cpuPointArena, cpuGraphData, cpuPercentage);
		add_graph_point(&memPointArena, memGraphData, memoryPercentage);

		graph_render(&cpuPointArena, cpuGraphData, cpuWin);
		graph_render(&memPointArena, memGraphData, memWin);

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
