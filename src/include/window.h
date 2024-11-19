#ifndef WINDOW_H
#define WINDOW_H

#include <ncurses.h>
#include <arena.h>

#define REFRESH_WIN(win) \
	do { \
		touchwin(win); \
		wrefresh(win); \
	} while (0) \

typedef enum _mt_window 
{
	CONTAINER_WIN = 0,
	CPU_WIN = 1,
	MEMORY_WIN = 2,
	PRC_WIN = 3
} mt_Window;

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
} WindowData;

typedef struct _display_items
{
	unsigned int windowCount;
	WindowData **windows;
} DisplayItems;

typedef struct _graph_point
{
	float percent;
	struct _graph_point *next;
} GraphPoint;

typedef struct _graph_data 
{
	unsigned short graphHeight;
	unsigned short graphWidth;
	unsigned short graphPointCount;
	GraphPoint *head;
} GraphData;

//
//		window_setup.c
//
//
DisplayItems * init_display_items(Arena *arena);
void init_windows(DisplayItems *di);
void init_window_dimens(DisplayItems *di);
void init_ncurses(WindowData *wd, SCREEN *screen);

//
//		graph.c
//
//
void graph_render(Arena *arena, GraphData *gd, WindowData *wd);
void add_graph_point(Arena *arena, GraphData *gd, float percentage);

#endif
