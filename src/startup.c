#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>

#include "include/startup.h"
#include "include/monitor.h"
#include "include/thread_safe_queue.h"
#include "include/window.h"
#include "include/thread.h"

typedef struct _ui_thread_args
{
	Arena *graphArena;
	Arena *memGraphArena;
	Arena *procArena;
	DisplayItems *di;
	ThreadSafeQueue *cpuQueue;
	ThreadSafeQueue *memQueue;
} UIThreadArgs;

typedef struct _io_thread_args
{
	Arena *cpuArena;
	Arena *memArena;
	Arena *processArena;
	ThreadSafeQueue *cpuQueue;
	ThreadSafeQueue *memQueue;
} IOThreadArgs;

volatile ProcessStats **procStats;

static Arena windowArena;
static Arena cpuArena;
static Arena memArena; 
static Arena cpuGraphArena;     // At some point come back and see
static Arena memoryGraphArena;  // if I can just use one graph arean
static Arena processArena;

static ThreadSafeQueue *cpuQueue;
static ThreadSafeQueue *memoryQueue;

static void * _ui_thread_run(void *arg);
static void * _io_thread_run(void *arg);
static void _get_input(DisplayItems *di);

void run() 
{
	FILE *tty = fopen("/dev/tty", "r+");
	SCREEN *screen = newterm(NULL, tty, tty);

	windowArena = a_new(2048);
	cpuArena = a_new(2048);
	memArena = a_new(2048);
	cpuGraphArena = a_new(2048);     // At some point come back and see
	memoryGraphArena = a_new(2048);  // if I can just use one graph arean
	processArena = a_new(512);

	DisplayItems *di = init_display_items(&windowArena);

	cpuQueue = a_alloc(
		&cpuArena,
		sizeof(ThreadSafeQueue),
		__alignof(ThreadSafeQueue)
	);

	memoryQueue = a_alloc(
		&memArena,
		sizeof(ThreadSafeQueue),
		__alignof(ThreadSafeQueue)
	);

	init_ncurses(di->windows[CONTAINER_WIN], screen);
	init_window_dimens(di);
	init_windows(di);
	
	UIThreadArgs uiArgs = 
	{
		.graphArena = &cpuGraphArena,
		.di = di,
		.procArena = &processArena,
		.cpuQueue = cpuQueue,
		.memGraphArena = &memoryGraphArena,
		.memQueue = memoryQueue
	};

	IOThreadArgs ioArgs = 
	{
		.cpuArena = &cpuArena,
		.memArena = &memArena,
		.processArena = &processArena,
		.cpuQueue = cpuQueue,
		.memQueue = memoryQueue
	};

	pthread_t ioThread;
	pthread_t ui_thread;

	mutex_init();
	condition_init();
	
	pthread_create(&ioThread, NULL, _io_thread_run, (void *)&ioArgs);
	pthread_create(&ui_thread, NULL, _ui_thread_run, (void *)&uiArgs);

	// wait for input to quit. Replace
	// with controls for process list later.
	_get_input(di);

	pthread_join(ioThread, NULL);
	pthread_join(ui_thread, NULL);
	mutex_destroy();
	condition_destroy();
	
	endwin();
	free(screen);
	fclose(tty);
}

void cleanup()
{
	QueueNode *memTmp;
	QueueNode *memHead = memoryQueue->head;

	while (memHead)
	{
		memTmp = memHead;
		memHead = memHead->next;

		free(memTmp);
	}

	QueueNode *cpuTmp;
	QueueNode *cpuHead = cpuQueue->head;

	while (cpuHead)
	{
		cpuTmp = cpuHead;
		cpuHead = cpuHead->next;

		free(cpuTmp);
	}

	a_free(&windowArena);
	a_free(&cpuArena);
	a_free(&memArena);
	a_free(&cpuGraphArena);
	a_free(&memoryGraphArena);
	a_free(&processArena);
}

void _get_input(DisplayItems *di)
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
	UIThreadArgs *args = (UIThreadArgs *)arg;

	run_ui(
		args->graphArena,
		args->memGraphArena,
		args->procArena,
		args->di,
		args->cpuQueue,
		args->memQueue
	);

	return NULL;
}

static void * _io_thread_run(void *arg)
{
	IOThreadArgs *args = (IOThreadArgs *)arg;

	run_io(
		args->cpuArena,
		args->memArena,
		args->processArena,
		args->cpuQueue,
		args->memQueue
	);

	return NULL;
}
