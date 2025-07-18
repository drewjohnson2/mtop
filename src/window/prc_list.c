#include <arena.h>
#include <bits/time.h>
#include <ctype.h>
#include <ncurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "../../include/window.h"
#include "../../include/mt_colors.h"
#include "../../include/monitor.h"
#include "../../include/sorting.h"
#include "../../include/static_text.h"
#include "../../include/tracked_stats.h"
#include "../../include/startup.h"

static char * _trim_lws(char *str);

void print_stats(
    ProcessListState *state,
    const WindowData *wd,
    ProcessStatsViewData **vd,
    s16 count
)
{
    if (vd == NULL || !mtopSettings->activeWindows[PRC_WIN]) return;
        
    const char *commandTitle = _text[24];
    const char *pidTitle = _text[25];
    const char *cpuTitle = _text[26];
    const char *memTitle = _text[27];
    const u8 dataOffsetY = 4;
    const u16 dataOffsetX = 2;
    const u16 prcTblHeaderY = 2;
    const u16 windowTitleX = 3;
    const u16 windowTitleY = 0;
    
    u16 pidPosX = wd->wWidth * .60;
    u16 cpuPosX = pidPosX + (wd->wWidth * .14);
    const u16 memPosX = cpuPosX + (wd->wWidth * .14);
    
    WINDOW *win = wd->window;
    
    u8 fitMem = wd->wWidth >= memPosX + strlen(memTitle);
    
    if (!fitMem) 
    {
    	pidPosX = wd->wWidth * .70;
    	cpuPosX = pidPosX + (wd->wWidth * .17);
    }
    
    u8 fitCpu = wd->wWidth >= cpuPosX + strlen(cpuTitle);
    
    //
    //
    //			Render Box and Header
    //
    //
    SET_COLOR(win, MT_PAIR_BOX);
    
    werase(win);
    box(win, 0, 0);


#ifdef DEBUG
    // May need this as a debug value again someday. Leaving for now.
    //char cmd = state->cmdBuffer ? state->cmdBuffer : '0'; 
    wattron(win, COLOR_PAIR(MT_PAIR_PRC_HEADER));
    mvwprintw(win, 
	windowTitleY, windowTitleX, 
	" 1st idx = %u, last = %u, selectedIndex = %u, maxidx = %u, toActive = %u, pc = %u, ap = %u",
	state->pageStartIdx, state->pageEndIdx, state->selectedIndex,
	state->count, state->timeoutActive, state->totalPages, state->activePage);

    wattroff(win, COLOR_PAIR(MT_PAIR_PRC_HEADER));
#else
    const u16 pageX = 3;
    const u16 pageY = wd->wHeight - 1;

    PRINTFC(win, windowTitleY, windowTitleX, " %s ", wd->windowTitle, MT_PAIR_PRC_HEADER);
    SET_COLOR(win, MT_PAIR_PRC_HEADER);
    mvwprintw(win, pageY, pageX, " Page %u/%u ", state->activePage + 1, state->totalPages); 
#endif

    wattron(win, A_BOLD);
    
    PRINTFC(win, prcTblHeaderY, dataOffsetX, "%s", commandTitle, MT_PAIR_PRC_TBL_HEADER);
    PRINTFC(win, prcTblHeaderY, pidPosX, "%s", pidTitle, MT_PAIR_PRC_TBL_HEADER);
    
    if (fitCpu) PRINTFC(win, prcTblHeaderY, cpuPosX, "%s", cpuTitle, MT_PAIR_PRC_TBL_HEADER);
    if (fitMem) PRINTFC(win, prcTblHeaderY, memPosX, "%s", memTitle, MT_PAIR_PRC_TBL_HEADER);
    
    for (size_t x = dataOffsetX; x < (size_t)wd->wWidth - dataOffsetX; x++)
    {
	PRINTFC(win, prcTblHeaderY + 1, x, "%c", '-', MT_PAIR_PRC_TBL_HEADER);
    }
    
    wattroff(win, A_BOLD);
    
    u8 posY = dataOffsetY;
    const u8 winDataOffset = 5;

    //
    //
    //			Render Process List
    //
    //
    for (u8 i = 0; i < wd->wHeight - winDataOffset && i < count; i++)
    {
    	const u16 idx = i + state->pageStartIdx;

	if (idx > state->count - 1) break;
    
    	MT_Color_Pairs pair = MT_PAIR_PRC_UNSEL_TEXT;
    
    	if (state->selectedIndex == idx)
    	{
	    pair = MT_PAIR_PRC_SEL_TEXT;

	    for (size_t y = dataOffsetX; y < (size_t)(wd->wWidth - dataOffsetX); y++)
		PRINTFC(win, posY, y, "%c", ' ', pair);
    	}
    
    	SET_COLOR(win, pair);
    	PRINTFC(win, posY, dataOffsetX, "%s", vd[idx]->command, pair);
    	PRINTFC(win, posY, pidPosX, "%d", vd[idx]->pid, pair);
    
	MT_Color_Pairs pctPair = 
	    (vd[idx]->cpuPercentage < 0.01 && pair != MT_PAIR_PRC_SEL_TEXT) ?
	    MT_PAIR_PRC_PCT_ZERO : 
	    pair;
    
	PRINTFC(win, posY, cpuPosX, "%.2f", vd[idx]->cpuPercentage, pctPair);
    
    	PRINTFC(win, posY++, memPosX, "%.2f", vd[idx]->memPercentage, pair);
    }
}

void set_prc_view_data(
    Arena *scratch,
    ProcessStatsViewData **vd,
    ProcessStats *curPrcs,
    ProcessStats *prevPrcs,
    u64 memTotal
)
{
    if (!mtopSettings->activeWindows[PRC_WIN]) return;

    for (size_t i = 0; i < curPrcs->count; i++)
    {
	float cpuPct = 0.0;
	float memPct = 0.0;
	
	ProcessList *target;
	ProcessList *cur = curPrcs->processes[i];
	ProcessList **match = bsearch(
	    &cur,
	    prevPrcs->processes,
	    prevPrcs->count,
	    sizeof(ProcessList *),
	    prc_pid_compare	
	);
	
	target = !match ? cur : *match;
	
	CALC_PRC_CPU_USAGE_PCT(
	    target,
	    cur,
	    cpuPct,
	    prevPrcs->cpuTimeAtSample,
	    curPrcs->cpuTimeAtSample
	);
	
	memPct = memTotal > 0 ? 
	    (cur->vmRss / (float)memTotal) * 100 :
	    0;
	
	vd[i] = a_alloc(
	    scratch,
	    sizeof(ProcessStatsViewData),
	    __alignof(ProcessStatsViewData)
	);
	
	vd[i]->pid = cur->pid;
	vd[i]->command = cur->procName;
	vd[i]->cpuPercentage = cpuPct;
	vd[i]->memPercentage = memPct;	
    }
}

void show_prc_info(ProcessStatsViewData *vd, ProcessInfo *info, const WindowData *wd) 
{
    Arena scratch = a_new(256);
    const u8 windowTitleY = 0;
    const u8 windowTitleX = 3;
    const u8 dataOffsetX = 2;
    const u8 maxTitleLength = strlen(trackedStats[1]);
    char prcInfoHeader[50];
    u8 posY = 2;
    u8 posX = 3;
    
    werase(wd->window);

    snprintf(
	prcInfoHeader,
	sizeof(prcInfoHeader),
	_text[38],
	info->pid,
	info->procName
    );

    wattron(wd->window, A_BOLD);
    PRINTFC(wd->window, posY++, 3, "%s", prcInfoHeader, MT_PAIR_PRC_STAT_TBL_HEADER);
    wattroff(wd->window, A_BOLD);

    for (size_t x = dataOffsetX; x < (size_t)wd->wWidth - dataOffsetX; x++)
    {
	PRINTFC(wd->window, posY, x, "%c", '-', MT_PAIR_PRC_STAT_TBL_HEADER);
    }

    posY++;

    wattron(wd->window, A_BOLD);
    PRINTFC(wd->window, posY, posX, "%s", _text[40], MT_PAIR_PRC_STAT_NM);
    wattroff(wd->window, A_BOLD);
    PRINTFC(wd->window, posY++, maxTitleLength + posX + 2, "%.2f", vd->cpuPercentage,
	    MT_PAIR_PRC_STAT_VAL);

    wattron(wd->window, A_BOLD);
    PRINTFC(wd->window, posY, posX, "%s", _text[41], MT_PAIR_PRC_STAT_NM);
    wattroff(wd->window, A_BOLD);
    PRINTFC(wd->window, posY++, maxTitleLength + posX + 2, "%.2f", vd->memPercentage,
	    MT_PAIR_PRC_STAT_VAL);

    for (size_t i = 0; i < 19; i++)
    {
	if (posY >= wd->wHeight - 4)
	{
	    posY = 4;
	    posX = wd->wWidth / 2;
	}

	char *str = a_strdup(&scratch, info->stats[i]);
	char *title = strtok(str, "\t");
	char *value = strtok(NULL, "\t");
	u8 valuePos = maxTitleLength + posX + 2;

	if (!title || !value) continue;

	value = _trim_lws(value);

	wattron(wd->window, A_BOLD);
	PRINTFC(wd->window, posY, posX, "%s\t", title, MT_PAIR_PRC_STAT_NM);
	wattroff(wd->window, A_BOLD);
	PRINTFC(wd->window, posY++, valuePos, "%s", value, MT_PAIR_PRC_STAT_VAL);
    }

    PRINTFC(wd->window, wd->wHeight - 2, 3, "%s", _text[36], MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 2, 5, "%s", _text[37], MT_PAIR_CTRL_TXT);
    SET_COLOR(wd->window, MT_PAIR_BOX);

    box(wd->window, 0, 0);

    PRINTFC(
	wd->window,
	windowTitleY,
	windowTitleX,
	" %s ",
	"Process Status",
	MT_PAIR_PRC_HEADER
    );

    a_free(&scratch);
}


static char * _trim_lws(char *str)
{
    while (*str && isspace((unsigned char)*str)) str++;

    return str;
}
