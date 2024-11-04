#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <arena.h>
#include <pthread.h>

#include "window/window.h"

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

void run_screen(Arena *cpuArena, Arena *graphArena, WINDOW_DATA *win, pthread_mutex_t *mutex,
				pthread_mutex_t *ncursesLock);
void run_ram_screen(Arena *ramArena, Arena *graphArena, WINDOW_DATA *win, pthread_mutex_t *mutex,
					pthread_mutex_t *ncursesLock);

#endif
