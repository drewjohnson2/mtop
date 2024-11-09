#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>

#include "include/startup/startup.h"
#include "include/util/shared_queue.h"
#include "include/window/window.h"
#include "include/window/window_setup.h"
#include "include/thread/ui_thread.h"
#include "include/thread/io_thread.h"
#include "include/thread/thread.h"

typedef struct _cpu_thread_args
{
	Arena *cpuArena;
	Arena *graphArena;
	WINDOW_DATA *cpuWin;
	SHARED_QUEUE *queue;
} CPU_THREAD_ARGS;

typedef struct _memory_thread_args
{
	Arena *memoryArena;
	Arena *graphArena;
	WINDOW_DATA *memWin;
	SHARED_QUEUE *queue;

} MEMORY_THREAD_ARGS;

typedef struct _io_thread_args
{
	Arena *cpuArena;
	Arena *memArena;
	SHARED_QUEUE *cpuQueue;
	SHARED_QUEUE *memQueue;
} IO_THREAD_ARGS;

static void * _cpu_thread_run(void *arg);
static void * _memory_thread_run(void *arg);
static void * _io_thread_run(void *arg);
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
	SHARED_QUEUE *q = a_alloc(&cpuArena, sizeof(SHARED_QUEUE), _Alignof(SHARED_QUEUE));
	SHARED_QUEUE *mq = a_alloc(&memArena, sizeof(SHARED_QUEUE), _Alignof(SHARED_QUEUE));

	init_ncurses(di->windows[CONTAINER_WIN], screen);
	init_window_dimens(di);
	init_windows(di);

	CPU_THREAD_ARGS uiArgs = 
	{
		.cpuArena = &cpuArena,
		.graphArena = &cpuGraphArena,
		.cpuWin = di->windows[CPU_WIN],
		.queue = q
	};

	MEMORY_THREAD_ARGS memoryArgs = 
	{
		.memoryArena = &memArena,
		.graphArena = &memoryGraphArena,
		.memWin = di->windows[MEMORY_WIN],
		.queue = mq
	};

	IO_THREAD_ARGS ioArgs = 
	{
		.cpuArena = &cpuArena,
		.memArena = &memArena,
		.cpuQueue = q,
		.memQueue = mq
	};

	// Using a mutex as a way to break
	// out of the infinite loop in the 
	// CPU/Memory threads, instead of
	// actually locking a shared resource.
	pthread_t ioThread;
	pthread_t cpuThread;
	pthread_t memoryThread;

	mutex_init();
	condition_init();
	
	pthread_mutex_lock(&runLock);
	pthread_create(&ioThread, NULL, _io_thread_run, (void *)&ioArgs);
	pthread_create(&cpuThread, NULL, _cpu_thread_run, (void *)&uiArgs);
	pthread_create(&memoryThread, NULL, _memory_thread_run, (void *)&memoryArgs);
	
	// wait for input to quit. Replace
	// with controls for process list later.
	_get_input(di);

	pthread_mutex_unlock(&runLock);
	pthread_join(ioThread, NULL);
	pthread_join(cpuThread, NULL);
	pthread_join(memoryThread, NULL);
	mutex_destroy();
	condition_destroy();
	
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
		args->graphArena,
		args->cpuWin,
		args->queue
	);

	return NULL;
}

static void * _memory_thread_run(void *arg)
{
	MEMORY_THREAD_ARGS *args = (MEMORY_THREAD_ARGS *)arg;

	run_memory_graph(
		args->graphArena,
		args->memWin,
		args->queue
	);

	return NULL;
}

static void * _io_thread_run(void *arg)
{
	IO_THREAD_ARGS *args = (IO_THREAD_ARGS *)arg;

	run_io(
		args->cpuArena,
		args->memArena,
		args->cpuQueue,
		args->memQueue
	);

	return NULL;
}
