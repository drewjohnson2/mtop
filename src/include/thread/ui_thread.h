#ifndef UI_THREAD_H
#define UI_THREAD_H

#include <arena.h>
#include <pthread.h>

#include "../window/window.h"

#define SHOULD_MERGE(mutex, cont) \
	do { \
		switch (pthread_mutex_trylock(mutex)) \
		{ \
			case 0: \
				pthread_mutex_unlock(mutex); \
				cont = 0; \
				break; \
			case EBUSY: \
				break; \
		} \
	} while(0) \

void run_ui(
	Arena *cpuArena,
	Arena *memArena,
	Arena *graphArena,
	WINDOW_DATA *cpuWin,
	WINDOW_DATA *memWin,
	pthread_mutex_t *mutex
);

#endif
