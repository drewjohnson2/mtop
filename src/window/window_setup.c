#include <locale.h>
#include <ncurses.h>
#include <arena.h>
#include <assert.h>

#include "../include/window.h"

DisplayItems * init_display_items(Arena *arena) 
{
	DisplayItems *di = a_alloc(arena, sizeof(DisplayItems), __alignof(DisplayItems));

	assert(di);

	di->windowCount = 4;

	di->windows = a_alloc(
		arena,
		sizeof(WindowData *) * di->windowCount,
		__alignof(WindowData *)
	);

	assert(di->windows);

	di->windows[CONTAINER_WIN] = a_alloc(
		arena, 
		sizeof(WindowData),
		__alignof(WindowData)
	);
	di->windows[CPU_WIN] = a_alloc(
		arena,
		sizeof(WindowData),
		__alignof(WindowData)
	);
	di->windows[MEMORY_WIN] = a_alloc(
		arena,
		sizeof(WindowData),
		__alignof(WindowData)
	);
	di->windows[PRC_WIN] = a_alloc(
		arena,
		sizeof(WindowData),
		__alignof(WindowData)
	);

	assert(di->windows[CONTAINER_WIN]);
	assert(di->windows[CPU_WIN]);
	assert(di->windows[MEMORY_WIN]);
	assert(di->windows[PRC_WIN]);

	return di;
}

void init_ncurses(WindowData *wd, SCREEN *screen)
{
	setlocale(LC_ALL, "");
	set_term(screen);
	start_color();
	raw();
	getmaxyx(stdscr, wd->wHeight, wd->wWidth);
	noecho();
	curs_set(0);
}

void init_window_dimens(DisplayItems *di) 
{
	WindowData *container = di->windows[CONTAINER_WIN];
	WindowData *cpuWin = di->windows[CPU_WIN];
	WindowData *memoryWin = di->windows[MEMORY_WIN];
	WindowData *prcWin = di->windows[PRC_WIN];


	container->windowX = 0;
	container->windowY = 0;

	cpuWin->paddingTop = 2;
	cpuWin->paddingBottom = 0;
	cpuWin->paddingLeft = 1;
	cpuWin->paddingRight = 1;
	cpuWin->windowTitle = "CPU Usage";

	memoryWin->paddingTop = 2;
	memoryWin->paddingBottom = 0;
	memoryWin->paddingLeft = 1;
	memoryWin->paddingRight = 0;
	memoryWin->windowTitle = "Memory Usage";
	
	prcWin->paddingTop = 2;
	prcWin->paddingBottom = 0;
	prcWin->paddingLeft = 1;
	prcWin->paddingRight = 0;
	prcWin->windowTitle = "Process List";

	// CPU win
	cpuWin->wWidth = container->wWidth - (cpuWin->paddingLeft + cpuWin->paddingRight);
	cpuWin->wHeight = (container->wHeight / 2) - (cpuWin->paddingTop + cpuWin->paddingBottom);
	cpuWin->windowX = cpuWin->paddingLeft;
	cpuWin->windowY = cpuWin->paddingTop;

	// Memory win
	memoryWin->wWidth = (container->wWidth / 2) - (memoryWin->paddingLeft + memoryWin->paddingRight);
	memoryWin->wHeight = (container->wHeight / 2);
	memoryWin->windowX = memoryWin->paddingLeft;
	memoryWin->windowY = cpuWin->wHeight + memoryWin->paddingTop;

	//Process Win
	prcWin->wWidth = (container->wWidth / 2) - (prcWin->paddingLeft + prcWin->paddingRight);
	prcWin->wHeight = (container->wHeight / 2); 
	prcWin->windowX = memoryWin->wWidth + prcWin->paddingLeft;
	prcWin->windowY = cpuWin->wHeight + prcWin->paddingTop;

}

void init_windows(DisplayItems *di) 
{
	WindowData *container = di->windows[CONTAINER_WIN];
	WindowData *cpuWin = di->windows[CPU_WIN];
	WindowData *memoryWin = di->windows[MEMORY_WIN];
	WindowData *prcWin = di->windows[PRC_WIN];

	 container->window = newwin(
		container->wHeight,
		container->wWidth,
		container->windowY,
		container->windowX
	);

	cpuWin->window = subwin(
		container->window,
		cpuWin->wHeight,
		cpuWin->wWidth,
		cpuWin->windowY,
		cpuWin->windowX
	);

	memoryWin->window = subwin(
		container->window,
		memoryWin->wHeight,
		memoryWin->wWidth,
		memoryWin->windowY,
		memoryWin->windowX
	);

	prcWin->window = subwin(
		container->window,
		prcWin->wHeight,
		prcWin->wWidth,
		prcWin->windowY,
		prcWin->windowX
	);

	assert(
		container->window &&
		cpuWin->window &&
		memoryWin->window &&
		prcWin->window
	);

	/*
		* just some test stuff, remove later
	*/
	init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);

	wbkgd(container->window, COLOR_PAIR(1));
	//
	// wattron(container->window, COLOR_PAIR(1));
	// box(cpuWin->window, 0, 0);
	// box(memoryWin->window, 0, 0);
	// box(prcWin->window, 0, 0);
	// wattroff(container->window, COLOR_PAIR(1));
	//
	// wmove(container->window, 1, 1);
	// wprintw(container->window, "A test of the windows");

	 //touchwin(container->window);
	 //wrefresh(container->window);
}
