#ifndef GRAPH_H
#define GRAPH_H

#include <ncurses.h>
#include <arena.h>

#include "window/window.h"

typedef struct _graph_point
{
	float percent;
	struct _graph_point *next;
} GRAPH_POINT;

typedef struct _graph_data 
{
	unsigned short graphHeight;
	unsigned short graphWidth;
	unsigned short graphPointCount;
	GRAPH_POINT *head;
} GRAPH_DATA;

void graph_render(GRAPH_DATA *gd, WINDOW_DATA *wd);
void add_graph_point(Arena *arena, GRAPH_DATA *gd, float percentage);

#endif
