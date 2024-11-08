#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>

#include "include/startup/startup.h"
#include "include/window/window.h"
#include "include/window/window_setup.h"
#include "include/thread/ui_thread.h"

pthread_mutex_t ncursesLock;

typedef struct _cpu_thread_args
{
	Arena *cpuArena;
	Arena *graphArena;
	WINDOW_DATA *cpuWin;
	pthread_mutex_t *runLock;
} CPU_THREAD_ARGS;

typedef struct _memory_thread_args
{
	Arena *memoryArena;
	Arena *graphArena;
	WINDOW_DATA *memWin;
	pthread_mutex_t *runLock;
} MEMORY_THREAD_ARGS;

static void * _cpu_thread_run(void *arg);
static void * _memory_thread_run(void *arg);
static void _get_input(DISPLAY_ITEMS *di);

void run() 
{
	FILE *tty = fopen("/dev/tty", "r+");
	SCREEN *screen = newterm(NULL, tty, tty);

	Arena windowArena = a_new(2048);
	Arena cpuArena = a_new(2048);
	Arena memArena = a_new(2048);
	Arena cpuGraphArena = a_new(2048);
	Arena memoryGraphArena = a_new(2048);
	DISPLAY_ITEMS *di = init_display_items(&windowArena);

	init_ncurses(di->windows[CONTAINER_WIN], screen);
	init_window_dimens(di);
	init_windows(di);

	CPU_THREAD_ARGS uiArgs = 
	{
		.cpuArena = &cpuArena,
		.graphArena = &cpuGraphArena,
		.cpuWin = di->windows[CPU_WIN],
	};

	MEMORY_THREAD_ARGS memoryArgs = 
	{
		.memoryArena = &memArena,
		.graphArena = &memoryGraphArena,
		.memWin = di->windows[MEMORY_WIN]
	};

	// Using a mutex as a way to break
	// out of the infinite loop in the 
	// CPU/Memory threads, instead of
	// actually locking a shared resource.
	pthread_t cpuThread;
	pthread_t memoryThread;
	pthread_mutex_t runLock;

	uiArgs.runLock = &runLock;
	memoryArgs.runLock = &runLock;

	pthread_mutex_init(&runLock, NULL);
	pthread_mutex_init(&ncursesLock, NULL);

	pthread_mutex_lock(&runLock);
	pthread_create(&cpuThread, NULL, _cpu_thread_run, (void *)&uiArgs);
	pthread_create(&memoryThread, NULL, _memory_thread_run, (void *)&memoryArgs);
	
	// wait for input to quit. Replace
	// with controls for process list later.
	//wgetch(di->windows[CONTAINER_WIN]->window);
	_get_input(di);

	pthread_mutex_unlock(&runLock);
	pthread_mutex_unlock(&ncursesLock);
	pthread_join(cpuThread, NULL);
	pthread_join(memoryThread, NULL);
	pthread_mutex_destroy(&runLock);
	pthread_mutex_destroy(&ncursesLock);

	endwin();
	free(screen);
	a_free(&windowArena);
	a_free(&cpuArena);
	a_free(&memArena);
	a_free(&cpuGraphArena);
	a_free(&memoryGraphArena);
	fclose(tty);
}

void _get_input(DISPLAY_ITEMS *di)
{
	WINDOW *win = di->windows[CONTAINER_WIN]->window;
	char ch;

	while ((ch = wgetch(win)))
	{
		switch (ch)
		{
			case 'q':
				return;
			default:
				continue;
		}
	}
}

static void * _cpu_thread_run(void *arg)
{
	CPU_THREAD_ARGS *args = (CPU_THREAD_ARGS *)arg;

	run_cpu_graph(
		args->cpuArena,
		args->graphArena,
		args->cpuWin,
		args->runLock
	);

	return NULL;
}

static void * _memory_thread_run(void *arg)
{
	MEMORY_THREAD_ARGS *args = (MEMORY_THREAD_ARGS *)arg;

	run_memory_graph(
		args->memoryArena,
		args->graphArena,
		args->memWin,
		args->runLock
	);

	return NULL;
}
