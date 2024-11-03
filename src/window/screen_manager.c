#include <arena.h>
#include <pthread.h>

#include "../include/cpu_monitor.h"
#include "../include/graph.h"
#include "../include/screen_manager.h"

void run_screen(
	Arena *cpuArena,
	Arena *graphArena,
	DISPLAY_ITEMS *di,
	pthread_mutex_t *mutex
)
{
	CPU_STATS *prevStats = fetch_cpu_stats(cpuArena);
	GRAPH_DATA *data = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	Arena pointArena = a_new(sizeof(GRAPH_POINT));
	float percentage;
	int cont = 1;

	while (cont)
	{
		napms(250);

		CPU_STATS *curStats = fetch_cpu_stats(cpuArena);

		percentage = calculate_cpu_usage(prevStats, curStats);

		add_graph_point(&pointArena, data, percentage);

		graph_render(&pointArena, data, di->windows[CPU_WIN]);

		prevStats = curStats;

		SHOULD_MERGE(mutex, cont);
	}

	a_free(&pointArena);
}
