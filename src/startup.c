#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>

#include "include/startup/startup.h"
#include "include/cpu_monitor.h"
#include "include/window/window.h"
#include "include/window/window_setup.h"
#include "include/screen_manager.h"

CPU_STATS *prevStats = NULL;
CPU_STATS *curStats = NULL;
RAM_STATS *memStats = NULL;

typedef struct _ui_thread_args
{
	Arena *cpuArena;
	Arena *memArena;
	Arena *graphArena;
	WINDOW_DATA *cpuWin;
	WINDOW_DATA *memWin;
	pthread_mutex_t *mutex;
	pthread_mutex_t *statsLock;
	pthread_cond_t *renderCondition;
} UI_THREAD_ARGS;

typedef struct _io_thread_args
{
	Arena *cpuArena;
	Arena *memArena;
	pthread_mutex_t *breakMutex;
	pthread_mutex_t *statsLock;
	pthread_cond_t *renderCondition;
} IO_THREAD_ARGS;

static void * _ui_thread_run(void *arg);
static void * _io_thread_run(void *arg);

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

	prevStats = fetch_cpu_stats(&cpuArena);
	curStats = fetch_cpu_stats(&cpuArena);

	UI_THREAD_ARGS cpuArgs = 
	{
		.cpuArena = &cpuArena,
		.memArena = &ramArena,
		.graphArena = &graphArena,
		.cpuWin = di->windows[CPU_WIN],
		.memWin = di->windows[MEMORY_WIN] 
	};

	IO_THREAD_ARGS ioArgs = {
		.cpuArena = &cpuArena,
		.memArena = &ramArena
	};

	// Using a mutex as a way to break
	// out of the infinite loop in the 
	// CPU/Memory threads, instead of
	// actually locking a shared resource.
	pthread_t cpuThread;
	pthread_t ioThread;

	pthread_cond_t renderCondition;

	pthread_mutex_t mutex;
	pthread_mutex_t statsLock;

	pthread_cond_init(&renderCondition, NULL);
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&statsLock, NULL);

	pthread_mutex_lock(&mutex);

	cpuArgs.mutex = &mutex;
	cpuArgs.statsLock = &statsLock;
	cpuArgs.renderCondition = &renderCondition;

	ioArgs.breakMutex = &mutex;
	ioArgs.statsLock = &statsLock;
	ioArgs.renderCondition = &renderCondition;

	pthread_create(&ioThread, NULL, _io_thread_run, (void *)&ioArgs);
	pthread_create(&cpuThread, NULL, _ui_thread_run, (void *)&cpuArgs);
	
	// wait for input to quit. Replace
	// with controls for process list later.
	wgetch(di->windows[CONTAINER_WIN]->window);

	pthread_mutex_unlock(&mutex);

	pthread_join(cpuThread, NULL);
	pthread_join(ioThread, NULL);

	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&statsLock);
	pthread_cond_destroy(&renderCondition);

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
		args->statsLock,
		args->renderCondition
	);

	return NULL;
}

static void * _io_thread_run(void *arg)
{
	IO_THREAD_ARGS *args = (IO_THREAD_ARGS *)arg;

	run_io(
		args->cpuArena,
		args->memArena,
		args->breakMutex,
		args->statsLock,
		args->renderCondition
	);

	return NULL;
}
