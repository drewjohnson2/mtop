#include <locale.h>
#include <ncurses.h>
#include <string.h>
#include <wchar.h>

#include "../include/graph.h"


void graph_render(GRAPH_DATA *gd, WINDOW_DATA *wd)
{
	if (!gd->head) return;

	WINDOW *win = wd->window;
	GRAPH_POINT *current = gd->head;
	short posX = wd->wWidth - gd->graphPointCount - 2;
	short posY = wd->wHeight - 2;

	if (posX < 1) posX = 1;

	werase(win);	
	box(win, 0, 0);

	while (current)
	{
		if (posX > wd->wWidth - 2) break;

		mvwprintw(win, 0, 3, "Percentage  = %.4f", current->percent * 100);

		int lineHeight = wd->wHeight * current->percent;
		
		lineHeight = lineHeight == 0 ? 1 : lineHeight;

		while (lineHeight--)
		{
			if (posY <= 0) break;

			
			// Extended ascii not playing nice
			// const wchar_t bullet = L'•';
			// wmove(win, posY--, posX);
			// waddnwstr(win, &bullet, -1);
			
			wmove(win, posY--, posX);
			wprintw(win, "%s", ":");
		}

		posY = wd->wHeight - 2;
		posX++;

		current = current->next;
	}

	if (gd->graphPointCount >= wd->wWidth)
	{
		// these are managed by the arena,
		// so I don't need to free. I'd like
		// to add a region_free() to the arena
		// library and maybe alloc each graph point
		// in a new region so I can free that way.
		// We'll see.
		GRAPH_POINT *tmp = gd->head;
		gd->head = gd->head->next;

		tmp = NULL;
		gd->graphPointCount--;
	}

	touchwin(win);
	wrefresh(win);
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
