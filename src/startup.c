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

typedef struct _cpu_thread_args
{
	Arena *cpuArena;
	Arena *graphArena;
	WINDOW_DATA *win;
	//DISPLAY_ITEMS *di;
	pthread_mutex_t *mutex;
	pthread_mutex_t *ncursesLock;
} CPU_THREAD_ARGS;

typedef struct _ram_thread_args
{
	Arena *ramArena;
	Arena *graphArena;
	WINDOW_DATA *win;
	//DISPLAY_ITEMS *di;
	pthread_mutex_t *mutex;
	pthread_mutex_t *ncursesLock;
} RAM_THREAD_ARGS;

static void * _cpu_thread_run(void *arg);
static void * _ram_thread_run(void *arg);

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

	CPU_THREAD_ARGS cpuArgs = 
	{
		.cpuArena = &cpuArena,
		.graphArena = &graphArena,
		.win = di->windows[CPU_WIN] 
	};

	RAM_THREAD_ARGS ramArgs = {
		.ramArena = &ramArena,
		.graphArena = &graphArena,
		.win = di->windows[MEMORY_WIN]
	};

	// Using a mutex as a way to break
	// out of the infinite loop in the 
	// CPU/Memory threads, instead of
	// actually locking a shared resource.
	pthread_t cpuThread;
	pthread_t ramThread;
	pthread_mutex_t mutex;
	pthread_mutex_t ncursesLock;

	cpuArgs.mutex = &mutex;
	ramArgs.mutex = &mutex;
	cpuArgs.ncursesLock = &ncursesLock;
	ramArgs.ncursesLock = &ncursesLock;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
	pthread_mutex_init(&ncursesLock, NULL);
	pthread_create(&cpuThread, NULL, _cpu_thread_run, (void *)&cpuArgs);
	pthread_create(&ramThread, NULL, _ram_thread_run, (void *)&ramArgs);
	
	// wait for input to quit. Replace
	// with controls for process list later.
	wgetch(di->windows[CONTAINER_WIN]->window);

	pthread_mutex_unlock(&mutex);
	pthread_mutex_unlock(&ncursesLock);
	pthread_join(cpuThread, NULL);
	pthread_join(ramThread, NULL);

	endwin();
	free(screen);
	a_free(&windowArena);
	a_free(&cpuArena);
	a_free(&ramArena);
	a_free(&graphArena);
	fclose(tty);
}

static void * _cpu_thread_run(void *arg)
{
	CPU_THREAD_ARGS *args = (CPU_THREAD_ARGS *)arg;

	run_screen(args->cpuArena, args->graphArena, args->win, args->mutex, args->ncursesLock);

	return NULL;
}

static void * _ram_thread_run(void *arg)
{
	RAM_THREAD_ARGS *args = (RAM_THREAD_ARGS *)arg;

	run_ram_screen(args->ramArena, args->graphArena, args->win, args->mutex, args->ncursesLock);

	return NULL;
}
