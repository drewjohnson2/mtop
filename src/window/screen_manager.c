#include <arena.h>
#include <errno.h>
#include <pthread.h>

#include "../include/ram_monitor.h"
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
	WINDOW_DATA *win,
	pthread_mutex_t *mutex,
	pthread_mutex_t *ncursesLock
)
{
	CPU_STATS *prevStats = fetch_cpu_stats(cpuArena);
	GRAPH_DATA *data = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	Arena pointArena = a_new(sizeof(GRAPH_POINT));
	float percentage;
	int cont = 1;

	while (!_merge_thread(mutex))
	{
		napms(350);

		CPU_STATS *curStats = fetch_cpu_stats(cpuArena);

		percentage = calculate_cpu_usage(prevStats, curStats);

		add_graph_point(&pointArena, data, percentage);

		pthread_mutex_lock(ncursesLock);
		graph_render(&pointArena, data, win);
		pthread_mutex_unlock(ncursesLock);

		prevStats = curStats;

		//SHOULD_MERGE(mutex, cont);
	}

	a_free(&pointArena);
}

void run_ram_screen(
	Arena *ramArena,
	Arena *graphArena,
	WINDOW_DATA *win,
	pthread_mutex_t *mutex,
	pthread_mutex_t *ncursesLock
)
{
	GRAPH_DATA *data = a_alloc(graphArena, sizeof(GRAPH_DATA), _Alignof(GRAPH_DATA));
	Arena pointArena = a_new(sizeof(GRAPH_POINT));
	float percentage;
	int cont = 1;

	while (!_merge_thread(mutex))
	{
		napms(350);

		RAM_STATS *stats = fetch_ram_stats(ramArena);

		percentage = calculate_ram_usage(stats);

		add_graph_point(&pointArena, data, percentage);

		pthread_mutex_lock(ncursesLock);
		graph_render(&pointArena, data, win);
		pthread_mutex_unlock(ncursesLock);

		//SHOULD_MERGE(mutex, cont);
	}

	a_free(&pointArena);
}
