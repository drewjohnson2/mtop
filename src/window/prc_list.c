#include <bits/time.h>
#include <ncurses.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "../../include/window.h"
#include "../../include/mt_colors.h"
#include "../../include/monitor.h"
#include "../../include/sorting.h"

void print_stats(
    ProcessListState *state,
    const WindowData *wd,
    ProcessStatsViewData **vd,
    s16 count
)
{
    if (vd == NULL) return;
        
    const char *commandTitle = _text[24];
    const char *pidTitle = _text[25];
    const char *cpuTitle = _text[26];
    const char *memTitle = _text[27];
    const u8 dataOffsetY = 4;
    const u16 dataOffsetX = 2;
    const u16 prcTblHeaderY = 2;
    const u16 windowTitleX = 3;
    const u16 windowTitleY = 0;
    const u16 pageX = 3;
    const u16 pageY = wd->wHeight - 1;
    
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
    char cmd = state->cmdBuffer ? state->cmdBuffer : '0';
    wattron(win, COLOR_PAIR(MT_PAIR_PRC_HEADER));
    mvwprintw(win, 
	windowTitleY, windowTitleX, 
	" 1st idx = %u, last = %u, selectedIndex = %u, maxidx = %u, toActive = %u, pc = %u, ap = %u",
	state->pageStartIdx, state->pageEndIdx, state->selectedIndex,
	state->count, state->timeoutActive, state->totalPages, state->activePage);
    wattroff(win, COLOR_PAIR(MT_PAIR_PRC_HEADER));
#else
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

    	const u8 isSelectedIndex = 
    		(state->selectedIndex - state->pageStartIdx) + dataOffsetY == posY;
    
    	MT_Color_Pairs pair = isSelectedIndex ?
	    MT_PAIR_PRC_SEL_TEXT :
	    MT_PAIR_PRC_UNSEL_TEXT;
    
    	if (pair == MT_PAIR_PRC_SEL_TEXT)
    	{
	    for (size_t y = dataOffsetX; y < wd->wWidth - dataOffsetX; y++)
		PRINTFC(win, posY, y, "%c", ' ', pair);
    	}
    
    	SET_COLOR(win, pair);
    	PRINTFC(win, posY, dataOffsetX, "%s", vd[idx]->command, pair);
    	PRINTFC(win, posY, pidPosX, "%d", vd[idx]->pid, pair);
    
    	if (fitCpu)
    	{
	    MT_Color_Pairs pctPair = vd[idx]->cpuPercentage < 0.01 && 
		pair != MT_PAIR_PRC_SEL_TEXT ? MT_PAIR_PRC_PCT_ZERO : pair;
    
	    PRINTFC(win, posY, cpuPosX, "%.2f", vd[idx]->cpuPercentage, pctPair);
    	}
    
    	if (fitMem) PRINTFC(win, posY++, memPosX, "%.2f", vd[idx]->memPercentage, pair);
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

void show_prc_info(ProcessInfo *info, const WindowData *wd) 
{
    const u8 windowTitleY = 0;
    const u8 windowTitleX = 3;

    SET_COLOR(wd->window, MT_PAIR_BOX);

    werase(wd->window);

    box(wd->window, 0, 0);

    PRINTFC(
	wd->window,
	windowTitleY,
	windowTitleX,
	" %s ",
	"Process Info",
	MT_PAIR_PRC_HEADER
    );
    PRINTFC(wd->window, 2, 3, "Name:\t%s", info->procName, MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, 3, 3, "State:\t%s", info->state, MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, 4, 3, "PPid:\t%d", info->pPid, MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, 5, 3, "VmPeak:\t%d kB", info->vmPeak, MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, 6, 3, "VmSize:\t%d kB", info->vmSize, MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, 7, 3, "VmLck:\t%d kB", info->vmLck, MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, 8, 3, "VmPin:\t%d kB", info->vmPin, MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, 9, 3, "VmHWM:\t%d kB", info->vmHWM, MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, 10, 3, "VmRSS:\t%d kB", info->vmRSS, MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, 11, 3, "Threads:\t%d", info->threads, MT_PAIR_PRC_UNSEL_TEXT);
}
