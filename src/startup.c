#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>

#include "include/startup/startup.h"
#include "include/window/window.h"
#include "include/window/window_setup.h"
#include "include/thread/ui_thread.h"

typedef struct _ui_thread_args
{
	Arena *cpuArena;
	Arena *memArena;
	Arena *graphArena;
	WINDOW_DATA *cpuWin;
	WINDOW_DATA *memWin;
	pthread_mutex_t *runLock;
} UI_THREAD_ARGS;

static void * _ui_thread_run(void *arg);
static void _get_input(DISPLAY_ITEMS *di);

void run() 
{
	FILE *tty = fopen("/dev/tty", "r+");
	SCREEN *screen = newterm(NULL, tty, tty);

	Arena windowArena = a_new(2048);
	Arena cpuArena = a_new(2048);
	Arena memArena = a_new(2048);
	Arena graphArena = a_new(2048);
	DISPLAY_ITEMS *di = init_display_items(&windowArena);

	init_ncurses(di->windows[CONTAINER_WIN], screen);
	init_window_dimens(di);
	init_windows(di);

	UI_THREAD_ARGS uiArgs = 
	{
		.cpuArena = &cpuArena,
		.memArena = &memArena,
		.graphArena = &graphArena,
		.cpuWin = di->windows[CPU_WIN],
		.memWin = di->windows[MEMORY_WIN] 
	};

	// Using a mutex as a way to break
	// out of the infinite loop in the 
	// CPU/Memory threads, instead of
	// actually locking a shared resource.
	pthread_t cpuThread;
	pthread_mutex_t runLock;

	uiArgs.runLock = &runLock;

	pthread_mutex_init(&runLock, NULL);
	pthread_mutex_lock(&runLock);
	pthread_create(&cpuThread, NULL, _ui_thread_run, (void *)&uiArgs);
	
	// wait for input to quit. Replace
	// with controls for process list later.
	//wgetch(di->windows[CONTAINER_WIN]->window);
	_get_input(di);

	pthread_mutex_unlock(&runLock);
	pthread_join(cpuThread, NULL);
	pthread_mutex_destroy(&runLock);

	endwin();
	free(screen);
	a_free(&windowArena);
	a_free(&cpuArena);
	a_free(&memArena);
	a_free(&graphArena);
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

static void * _ui_thread_run(void *arg)
{
	UI_THREAD_ARGS *args = (UI_THREAD_ARGS *)arg;

	run_ui(
		args->cpuArena,
		args->memArena,
		args->graphArena,
		args->cpuWin,
		args->memWin,
		args->runLock
	);

	return NULL;
}
