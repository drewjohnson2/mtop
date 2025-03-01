#include <locale.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include <wchar.h>
#include <arena.h>
#include <unistd.h>
#include <assert.h>

#include "../../include/window.h"
#include "../../include/mt_colors.h"

s8 graph_render(
    Arena *arena,
    GraphData *gd,
    const WindowData *wd,
    MT_Color_Pairs gpColor,
    MT_Color_Pairs headerColor
)
{
    if (!gd->head) return 1;
    
    assert(gd && wd);
    
    WINDOW *win = wd->window;
    GraphPoint *current = gd->head;
    GraphPoint *last = current;
    s16 posX = wd->wWidth - gd->graphPointCount - 2;
    s16 posY = wd->wHeight - 2;
    
    if (posX < 2) posX = 2;
    
    werase(win);	
    
    while (current)
    {
	if (posX > wd->wWidth - 3) break;
    
	s16 lineHeight = (wd->wHeight - 3) * current->percent;
	const char dataChar = current->percent * 100 == 0 ? '.' : '|';
			
	lineHeight = lineHeight == 0 ? 1 : lineHeight;
	
	while (lineHeight--)
	{
	    if (posY <= 0) break;
		
	    // Extended ascii not playing nice
	    // const wchar_t bullet = L'•';
	    // wmove(win, posY--, posX);
	    // waddnwstr(win, &bullet, -1);
		
	    PRINTFC(win, posY--, posX, "%c", dataChar, gpColor);
	}
	
	posY = wd->wHeight - 2;
	posX++;

	SET_COLOR(wd->window, MT_PAIR_BOX);
	box(win, 0, 0);

#ifdef DEBUG
	PRINTFC(win, 0, 3, " Percentage  = %.4f ", current->percent * 100, 
	    headerColor);
	PRINTFC(win, 0, 35, " Arena Regions Alloc'd  = %zu ", arena->regionsAllocated,
	    headerColor);
#else 
	PRINTFC(win, 0, 3, " %s ", wd->windowTitle, headerColor);
#endif
	last = current;	
	current = current->next;
    }

    const s8 pctLabel = (s8)(last->percent * 100);
    const s16 pctPadLeft = pctLabel < 10 ?
	wd->wWidth - 5 :
	wd->wWidth - 6;

    PRINTFC(win, 1, pctPadLeft, " %d%% ", pctLabel, headerColor);

    // NOTE: I've created an arena specifically for 
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
    	gd->head = gd->head->next;
    	gd->graphPointCount--;
    
    	r_free_head(arena);
    }
    
    return 0;
}

// NOTE: I'm allocating graph points in a different arena
// than the graph data. Even though GraphPoint is a member of
// GraphData it's important that I allocate those items in a different
// struct so that the GraphPoint linked list can be freed and doesn't grow without
// bounds.You could imagine the issues with trying to allocate both of these
// types on the same arena. This is probably a really dumb way to do it,
// but idgaf.
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
