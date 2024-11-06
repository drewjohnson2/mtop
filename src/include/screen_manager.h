#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <arena.h>
#include <pthread.h>

#include "startup/startup.h"
#include "window/window.h"
#include "../include/cpu_monitor.h"
#include "../include/ram_monitor.h"

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
);

void run_io(
	Arena *cpuArena,
	Arena *memArena,
	pthread_mutex_t *breakMutex,
	pthread_mutex_t *statsLock,
	pthread_cond_t *fileCond,
	shared_data **sd
);

#endif
