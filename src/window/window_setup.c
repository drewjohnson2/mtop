#include <locale.h>
#include <ncurses.h>
#include <arena.h>

#include "../include/window/window_setup.h"
#include "../include/window/window.h"

DISPLAY_ITEMS * init_display_items(Arena *arena) 
{
	DISPLAY_ITEMS *di = a_alloc(arena, sizeof(DISPLAY_ITEMS), __alignof(DISPLAY_ITEMS));

	di->windowCount = 4;

	di->windows = a_alloc(
		arena,
		sizeof(DISPLAY_ITEMS *) * di->windowCount,
		__alignof(DISPLAY_ITEMS *)
	);
	di->windows[CONTAINER_WIN] = a_alloc(
		arena, 
		sizeof(DISPLAY_ITEMS),
		__alignof(DISPLAY_ITEMS)
	);
	di->windows[CPU_WIN] = a_alloc(
		arena,
		sizeof(DISPLAY_ITEMS),
		__alignof(DISPLAY_ITEMS)
	);
	di->windows[MEMORY_WIN] = a_alloc(
		arena,
		sizeof(DISPLAY_ITEMS),
		__alignof(DISPLAY_ITEMS)
	);
	di->windows[PRC_WIN] = a_alloc(
		arena,
		sizeof(DISPLAY_ITEMS),
		__alignof(DISPLAY_ITEMS)
	);

	return di;
}

void init_ncurses(WINDOW_DATA *wd, SCREEN *screen)
{
	setlocale(LC_ALL, "");
	set_term(screen);
	start_color();
	raw();
	getmaxyx(stdscr, wd->wHeight, wd->wWidth);
	noecho();
	curs_set(0);
}

void init_window_dimens(DISPLAY_ITEMS *di) 
{
	WINDOW_DATA *container = di->windows[CONTAINER_WIN];
	WINDOW_DATA *cpuWin = di->windows[CPU_WIN];
	WINDOW_DATA *memoryWin = di->windows[MEMORY_WIN];
	WINDOW_DATA *prcWin = di->windows[PRC_WIN];


	container->windowX = 0;
	container->windowY = 0;

	cpuWin->paddingTop = 2;
	cpuWin->paddingBottom = 0;
	cpuWin->paddingLeft = 1;
	cpuWin->paddingRight = 1;
	
	memoryWin->paddingTop = 2;
	memoryWin->paddingBottom = 0;
	memoryWin->paddingLeft = 1;
	memoryWin->paddingRight = 0;
	
	prcWin->paddingTop = 2;
	prcWin->paddingBottom = 0;
	prcWin->paddingLeft = 1;
	prcWin->paddingRight = 0;

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

void init_windows(DISPLAY_ITEMS *di) 
{
	WINDOW_DATA *container = di->windows[CONTAINER_WIN];
	WINDOW_DATA *cpuWin = di->windows[CPU_WIN];
	WINDOW_DATA *memoryWin = di->windows[MEMORY_WIN];
	WINDOW_DATA *prcWin = di->windows[PRC_WIN];

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

	/*
		* just some test stuff, remove later
	*/
	init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);
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
