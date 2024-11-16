#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>

#include "include/startup/startup.h"
#include "include/monitor/proc_monitor.h"
#include "include/util/shared_queue.h"
#include "include/window/window.h"
#include "include/window/window_setup.h"
#include "include/thread/ui_thread.h"
#include "include/thread/io_thread.h"
#include "include/thread/thread.h"
#include "include/util/ui_utils.h"

typedef struct _cpu_thread_args
{
	Arena *graphArena;
	Arena *memGraphArena;
	DISPLAY_ITEMS *di;
	SHARED_QUEUE *cpuQueue;
	SHARED_QUEUE *memQueue;
	PROC_STATS **procStats;


} GRAPH_THREAD_ARGS;

typedef struct _io_thread_args
{
	Arena *cpuArena;
	Arena *memArena;
	Arena *processArena;
	SHARED_QUEUE *cpuQueue;
	SHARED_QUEUE *memQueue;
	PROC_STATS **procStats;
} IO_THREAD_ARGS;

static void * _ui_thread_run(void *arg);
static void * _io_thread_run(void *arg);
static void _get_input(DISPLAY_ITEMS *di);

void run() 
{
	FILE *tty = fopen("/dev/tty", "r+");
	SCREEN *screen = newterm(NULL, tty, tty);

	Arena windowArena = a_new(2048);
	Arena cpuArena = a_new(2048);
	Arena memArena = a_new(2048);
	Arena cpuGraphArena = a_new(2048);     // At some point come back and see
	Arena memoryGraphArena = a_new(2048);  // if I can just use one graph arean
	Arena processArena = a_new(512);
	DISPLAY_ITEMS *di = init_display_items(&windowArena);

	SHARED_QUEUE *cpuQueue = a_alloc(
		&cpuArena,
		sizeof(SHARED_QUEUE),
		__alignof(SHARED_QUEUE)
	);

	SHARED_QUEUE *memoryQueue = a_alloc(
		&memArena,
		sizeof(SHARED_QUEUE),
		__alignof(SHARED_QUEUE)
	);

	PROC_STATS **procStats = get_processes(&processArena, proc_name_compare);

	init_ncurses(di->windows[CONTAINER_WIN], screen);
	init_window_dimens(di);
	init_windows(di);
	
	GRAPH_THREAD_ARGS uiArgs = 
	{
		.graphArena = &cpuGraphArena,
		.di = di,
		.cpuQueue = cpuQueue,
		.memGraphArena = &memoryGraphArena,
		.memQueue = memoryQueue,
		.procStats = procStats
	};

	IO_THREAD_ARGS ioArgs = 
	{
		.cpuArena = &cpuArena,
		.memArena = &memArena,
		.processArena = &processArena,
		.cpuQueue = cpuQueue,
		.memQueue = memoryQueue,
		.procStats = procStats
	};

	pthread_t ioThread;
	pthread_t ui_thread;

	mutex_init();
	condition_init();
	
	pthread_mutex_lock(&runLock);
	pthread_create(&ioThread, NULL, _io_thread_run, (void *)&ioArgs);
	pthread_create(&ui_thread, NULL, _ui_thread_run, (void *)&uiArgs);

	
	// wait for input to quit. Replace
	// with controls for process list later.
	_get_input(di);

	pthread_mutex_unlock(&runLock);
	pthread_join(ioThread, NULL);
	pthread_join(ui_thread, NULL);
	mutex_destroy();
	condition_destroy();
	
	endwin();
	free(screen);
	a_free(&windowArena);
	a_free(&cpuArena);
	a_free(&memArena);
	a_free(&cpuGraphArena);
	a_free(&memoryGraphArena);
	a_free(&processArena);
	fclose(tty);
}

void _get_input(DISPLAY_ITEMS *di)
{
	char ch;
	WINDOW *win = di->windows[CONTAINER_WIN]->window;
	
	while ((ch = wgetch(win)))
	{
		switch (ch)
		{
			case 'q':
				SHUTDOWN_FLAG = 1;
				return;
			default:
				continue;
		}
	}
}

static void * _ui_thread_run(void *arg)
{
	GRAPH_THREAD_ARGS *args = (GRAPH_THREAD_ARGS *)arg;

	run_ui(
		args->graphArena,
		args->memGraphArena,
		args->di,
		args->cpuQueue,
		args->memQueue,
		args->procStats
	);

	return NULL;
}

static void * _io_thread_run(void *arg)
{
	IO_THREAD_ARGS *args = (IO_THREAD_ARGS *)arg;

	run_io(
		args->cpuArena,
		args->memArena,
		args->processArena,
		args->cpuQueue,
		args->memQueue,
		args->procStats
	);

	return NULL;
}
