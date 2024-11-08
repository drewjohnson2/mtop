#include <locale.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include <wchar.h>
#include <arena.h>

#include "../include/startup/startup.h"
#include "../include/graph.h"

void graph_render(Arena *arena, GRAPH_DATA *gd, WINDOW_DATA *wd)
{
	if (!gd->head) return;

	pthread_mutex_lock(&ncursesLock);

	napms(200);

	WINDOW *win = wd->window;
	GRAPH_POINT *current = gd->head;
	short posX = wd->wWidth - gd->graphPointCount - 2;
	short posY = wd->wHeight - 2;

	if (posX < 1) posX = 1;

	wattron(wd->window, COLOR_PAIR(2));

	werase(win);	
	box(win, 0, 0);

	while (current)
	{
		if (posX > wd->wWidth - 2) break;

		mvwprintw(win, 0, 3, " Percentage  = %.4f ", current->percent * 100);
		mvwprintw(win, 0, 35, " Arena Regions Alloc'd  = %zu ", arena->regionsAllocated);

		int lineHeight = wd->wHeight * current->percent;
		
		lineHeight = lineHeight == 0 ? 1 : lineHeight;

		char dataChar = current->percent * 100 == 0 ? '.' : '|';

		while (lineHeight--)
		{
			if (posY <= 0) break;
			
			// Extended ascii not playing nice
			// const wchar_t bullet = L'•';
			// wmove(win, posY--, posX);
			// waddnwstr(win, &bullet, -1);
			
			wmove(win, posY--, posX);
			wprintw(win, "%c", dataChar);
		}

		posY = wd->wHeight - 2;
		posX++;

		current = current->next;
	}

	// I've created an arena specifically for 
	// graph points. If I let the graph points
	// linked list grow without bounds we'll
	// eventually run out of memory. So I make 
	// each region on the arena the size of 
	// GRAPH_POINT, and when a graph point
	// is outside of render bounds I free it.
	// This is essentially just freeing the head
	// of the region linked list. Then we set the
	// head of the point linked list to NULL.
	if (gd->graphPointCount >= wd->wWidth)
	{
		GRAPH_POINT *tmp = gd->head;
		gd->head = gd->head->next;

		tmp = NULL;
		gd->graphPointCount--;

		r_free_head(arena);
	}

	wattroff(win, COLOR_PAIR(2));

	pthread_mutex_unlock(&ncursesLock);
}

void add_graph_point(Arena *arena, GRAPH_DATA *gd, float percentage)
{
	GRAPH_POINT *gp = a_alloc(arena, sizeof(GRAPH_POINT), _Alignof(GRAPH_POINT));
	
	gp->percent = percentage;
	
	gd->graphPointCount++;

	if (gd->head == NULL) 
	{
		gd->head = gp;

		return;
	}

	GRAPH_POINT *tmp = gd->head;

	while (tmp->next != NULL) tmp = tmp->next;

	tmp->next = gp;
}
