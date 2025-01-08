#include <locale.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include <wchar.h>
#include <arena.h>
#include <unistd.h>
#include <assert.h>

#include "../include/window.h"
#include "../include/mt_colors.h"

s8 graph_render(Arena *arena, GraphData *gd, WindowData *wd)
{
    if (!gd->head) return 1;
    
    assert(gd && wd);
    
    WINDOW *win = wd->window;
    GraphPoint *current = gd->head;
    s16 posX = wd->wWidth - gd->graphPointCount - 2;
    s16 posY = wd->wHeight - 2;
    
    if (posX < 2) posX = 2;
    
    SET_COLOR(wd->window, MT_PAIR_BOX);
    werase(win);	
    box(win, 0, 0);
    SET_COLOR(wd->window, MT_PAIR_CPU_GP);
    
    while (current)
    {
	if (posX > wd->wWidth - 3) break;
    
	s8 pctLabel = (s8)(current->percent * 100);

#ifdef DEBUG
	PRINTFC(win, 0, 3, " Percentage  = %.4f ", current->percent * 100, 
	    MT_PAIR_CPU_HEADER);
	PRINTFC(win, 0, 35, " Arena Regions Alloc'd  = %zu ", arena->regionsAllocated,
	    MT_PAIR_CPU_HEADER);
#else 
	PRINTFC(win, 0, 3, " %s ", wd->windowTitle, MT_PAIR_CPU_HEADER);
#endif
	s16 lineHeight = (wd->wHeight - 3) * current->percent;
		
	lineHeight = lineHeight == 0 ? 1 : lineHeight;

	char dataChar = current->percent * 100 == 0 ? '.' : '|';
	s16 pctPadLeft = pctLabel < 10 ?
		wd->wWidth - 5 :
		wd->wWidth - 6;
	
	PRINTFC(win, 1, pctPadLeft, " %d%% ", pctLabel, MT_PAIR_CPU_HEADER);
	
	while (lineHeight--)
	{
		if (posY <= 0) break;
		
		// Extended ascii not playing nice
		// const wchar_t bullet = L'•';
		// wmove(win, posY--, posX);
		// waddnwstr(win, &bullet, -1);
		
		PRINTFC(win, posY--, posX, "%c", dataChar, MT_PAIR_CPU_GP);
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
    // GraphPoint, and when a graph point
    // is outside of render bounds I free it.
    // This is essentially just freeing the head
    // of the region linked list. Then we set the
    // head of the point linked list to NULL.
    if (gd->graphPointCount >= wd->wWidth)
    {
    	GraphPoint *tmp = gd->head;
    	gd->head = gd->head->next;
    
    	tmp = NULL;
    	gd->graphPointCount--;
    
    	r_free_head(arena);
    }
    
    UNSET_COLOR(wd->window, MT_PAIR_CPU_GP);
    
    return 0;
}

s8 add_graph_point(Arena *arena, GraphData *gd, float percentage)
{
    assert(arena && gd);
    
    GraphPoint *gp = a_alloc(arena, sizeof(GraphPoint), __alignof(GraphPoint));
    
    assert(gp);
    
    gp->percent = percentage;
    
    gd->graphPointCount++;
    
    if (gd->head == NULL) 
    {
	gd->head = gp;
    
    	return 0;
    }
    
    GraphPoint *tmp = gd->head;
    
    while (tmp->next != NULL) tmp = tmp->next;
    
    tmp->next = gp;
    
    return 0;
}
