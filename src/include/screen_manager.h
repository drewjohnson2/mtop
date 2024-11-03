#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <arena.h>
#include <pthread.h>

#include "window/window.h"

void run_screen(Arena *cpuArena, Arena *graphArena, DISPLAY_ITEMS *di, pthread_mutex_t *mutex);

#endif
