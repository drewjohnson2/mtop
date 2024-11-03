#ifndef WINDOW_H
#define WINDOW_H

#include <ncurses.h>

typedef enum _mt_window 
{
	CONTAINER_WIN = 0,
	CPU_WIN = 1,
	MEMORY_WIN = 2,
	PRC_WIN = 3
} MT_WINDOW;

typedef struct _window_data
{
	WINDOW *window;
	unsigned short wHeight, wWidth;
	unsigned short windowX, windowY;
	unsigned short paddingTop;
	unsigned short paddingBottom;
	unsigned short paddingRight;
	unsigned short paddingLeft;
	char *windowTitle;
} WINDOW_DATA;

typedef struct _display_items
{
	unsigned int windowCount;
	WINDOW_DATA **windows;
} DISPLAY_ITEMS;

#endif
