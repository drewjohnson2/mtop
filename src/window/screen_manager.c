#include <errno.h>
#include <pthread.h>

#include "../include/cpu_monitor.h"
#include "../include/graph.h"
#include "../include/screen_manager.h"

static int _merge_thread(pthread_mutex_t *mutex)
{
	switch (pthread_mutex_trylock(mutex))
	{
		case 0:
			pthread_mutex_unlock(mutex);
			return 1;

		case EBUSY:
			return 0;
	}

	return 1;
}

void run_screen(
	Arena *cpuArena,
	Arena *graphArena,
	DISPLAY_ITEMS *di,
	pthread_mutex_t *mutex
)
{
	CPU_STATS *prevStats = fetch_cpu_stats(cpuArena);
	GRAPH_DATA *data = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	float percentage;

	while (!_merge_thread(mutex))
	{
		napms(150);

		CPU_STATS *curStats = fetch_cpu_stats(cpuArena);

		percentage = calculate_cpu_usage(prevStats, curStats);

		add_graph_point(graphArena, data, percentage);

		graph_render(data, di->windows[CPU_WIN]);

		prevStats = curStats;
	}
}
