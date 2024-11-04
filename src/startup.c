#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>

#include "include/startup/startup.h"
#include "include/ram_monitor.h"
#include "include/window/window.h"
#include "include/window/window_setup.h"
#include "include/screen_manager.h"

typedef struct _ui_thread_args
{
	Arena *cpuArena;
	Arena *memArena;
	Arena *graphArena;
	WINDOW_DATA *cpuWin;
	WINDOW_DATA *memWin;
	pthread_mutex_t *mutex;
	pthread_mutex_t *ncursesLock;
} UI_THREAD_ARGS;

static void * _ui_thread_run(void *arg);

void run() 
{
	FILE *tty = fopen("/dev/tty", "r+");
	SCREEN *screen = newterm(NULL, tty, tty);

	Arena windowArena = a_new(2048);
	Arena cpuArena = a_new(2048);
	Arena ramArena = a_new(2048);
	Arena graphArena = a_new(2048);
	DISPLAY_ITEMS *di = init_display_items(&windowArena);

	init_ncurses(di->windows[CONTAINER_WIN], screen);
	init_window_dimens(di);
	init_windows(di);

	UI_THREAD_ARGS cpuArgs = 
	{
		.cpuArena = &cpuArena,
		.memArena = &ramArena,
		.graphArena = &graphArena,
		.cpuWin = di->windows[CPU_WIN],
		.memWin = di->windows[MEMORY_WIN] 
	};

	// Using a mutex as a way to break
	// out of the infinite loop in the 
	// CPU/Memory threads, instead of
	// actually locking a shared resource.
	pthread_t cpuThread;
	pthread_mutex_t mutex;
	pthread_mutex_t ncursesLock;

	cpuArgs.mutex = &mutex;
	cpuArgs.ncursesLock = &ncursesLock;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
	pthread_mutex_init(&ncursesLock, NULL);
	pthread_create(&cpuThread, NULL, _ui_thread_run, (void *)&cpuArgs);
	
	// wait for input to quit. Replace
	// with controls for process list later.
	wgetch(di->windows[CONTAINER_WIN]->window);

	pthread_mutex_unlock(&mutex);
	pthread_mutex_unlock(&ncursesLock);
	pthread_join(cpuThread, NULL);

	endwin();
	free(screen);
	a_free(&windowArena);
	a_free(&cpuArena);
	a_free(&ramArena);
	a_free(&graphArena);
	fclose(tty);
}

static void * _ui_thread_run(void *arg)
{
	UI_THREAD_ARGS *args = (UI_THREAD_ARGS *)arg;

	run_screen(
		args->cpuArena,
		args->memArena,
		args->graphArena,
		args->cpuWin,
		args->memWin,
		args->mutex,
		args->ncursesLock
	);

	return NULL;
}
