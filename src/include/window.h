#ifndef WINDOW_H
#define WINDOW_H

#include <ncurses.h>
#include <arena.h>

#include "mt_type_defs.h"

#define REFRESH_WIN(win) \
	do { \
		touchwin(win); \
		wrefresh(win); \
	} while (0) \


typedef enum _mt_window 
{
#define DEFINE_WINDOWS(winName, enumName) enumName,
	CONTAINER_WIN,
#include "../include/tables/window_def_table.h"
	WINDOW_ID_MAX
#undef DEFINE_WINDOWS
} mt_Window;


typedef struct _window_data
{
	WINDOW *window;
	u16 wHeight, wWidth;
	u16 windowX, windowY;
	u16 paddingTop;
	u16 paddingBottom;
	u16 paddingRight;
	u16 paddingLeft;
	char *windowTitle;
} WindowData;

typedef struct _display_items
{
	size_t windowCount;
	WindowData **windows;
} DisplayItems;

typedef struct _graph_point
{
	float percent;
	struct _graph_point *next;
} GraphPoint;

typedef struct _graph_data 
{
	u16 graphHeight; // as far as I'm aware 
	u16 graphWidth; // these values are unused.
	size_t graphPointCount;
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
s8 graph_render(Arena *arena, GraphData *gd, WindowData *wd);
s8 add_graph_point(Arena *arena, GraphData *gd, float percentage);

#endif
