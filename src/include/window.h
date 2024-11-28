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
	u32 windowCount;
	WindowData **windows;
} DisplayItems;

typedef struct _graph_point
{
	float percent;
	struct _graph_point *next;
} GraphPoint;

typedef struct _graph_data 
{
	u16 graphHeight;
	u16 graphWidth;
	u16 graphPointCount;
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
int graph_render(Arena *arena, GraphData *gd, WindowData *wd);
int add_graph_point(Arena *arena, GraphData *gd, float percentage);

#endif
