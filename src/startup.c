#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>
#include <pthread.h>

#include "include/startup/startup.h"
#include "include/window/window_setup.h"
#include "include/screen_manager.h"

typedef struct _cpu_thread_args
{
	Arena *cpuArena;
	Arena *graphArena;
	DISPLAY_ITEMS *di;
	pthread_mutex_t *mutex;
} CPU_THREAD_ARGS;

static void * _cpu_thread_run(void *arg);

void run() 
{
	FILE *tty = fopen("/dev/tty", "r+");
	SCREEN *screen = newterm(NULL, tty, tty);

	Arena windowArena = a_new(2048);
	Arena cpuArena = a_new(2048);
	Arena graphArena = a_new(2048);
	DISPLAY_ITEMS *di = init_display_items(&windowArena);

	init_ncurses(di->windows[CONTAINER_WIN], screen);
	init_window_dimens(di);
	init_windows(di);

	CPU_THREAD_ARGS cpuArgs = 
	{
		.cpuArena = &cpuArena,
		.graphArena = &graphArena,
		.di = di
	};

	// Using a mutex as a way to break
	// out of the infinite loop in the 
	// CPU/Memory threads, instead of
	// actually locking a shared resource.
	pthread_t cpuThread;
	pthread_mutex_t mutex;

	cpuArgs.mutex = &mutex;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
	pthread_create(&cpuThread, NULL, _cpu_thread_run, (void *)&cpuArgs);
	
	// wait for input to quit. Replace
	// with controls for process list later.
	wgetch(di->windows[CONTAINER_WIN]->window);

	pthread_mutex_unlock(&mutex);
	pthread_join(cpuThread, NULL);

	endwin();
	free(screen);
	a_free(&windowArena);
	a_free(&cpuArena);
	a_free(&graphArena);
	fclose(tty);
}

static void * _cpu_thread_run(void *arg)
{
	CPU_THREAD_ARGS *args = (CPU_THREAD_ARGS *)arg;

	run_screen(args->cpuArena, args->graphArena, args->di, args->mutex);

	return NULL;
}
