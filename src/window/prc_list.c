#include <bits/time.h>
#include <signal.h>
#include <ncurses.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "../include/window.h"
#include "../include/mt_colors.h"
#include "../include/monitor.h"
#include "../include/sorting.h"
#include "../include/thread.h"

void read_input(WINDOW *win, ProcessListState *state, ProcessStatsViewData **vd)
{
    char ch = wgetch(win);
    u64 timeElapsedMs;
    struct timespec timeoutCurrent;
    
    if (state->timeoutActive)
    {
	clock_gettime(CLOCK_REALTIME, &timeoutCurrent);
    
    	timeElapsedMs = (timeoutCurrent.tv_sec - state->timeoutStart.tv_sec) * 1000
    	 + (timeoutCurrent.tv_nsec - state->timeoutStart.tv_nsec) 
    		/ 1000000;
    
    	if (timeElapsedMs > INPUT_TIMEOUT_MS)
    	{
	    state->cmdBuffer = '\0';
	    state->timeoutActive = 0;
    	}
    }
    
    if (ch == -1) return;
    
    switch (ch)
    {
	case 'j':
	    state->selectedIndex = state->selectedIndex < state->maxIndex ?
	    	state->selectedIndex + 1 : 
	    	state->selectedIndex;
	    
	    if (state->selectedIndex > state->lastIndexDisplayed &&
	    	state->selectedIndex <= state->maxIndex)
	    {
	    	state->firstIndexDisplayed++;
	    	state->lastIndexDisplayed++;
	    }
    
	    return;
    	case 'k':
	    state->selectedIndex = state->selectedIndex > 0 ?
		state->selectedIndex - 1 :
	    	state->selectedIndex;
	    
	    if (state->selectedIndex >= 0 &&
	    	state->selectedIndex < state->firstIndexDisplayed)
	    {
	    	state->firstIndexDisplayed--;
	    	state->lastIndexDisplayed--;
	    }
	    
	    return;
	case 'n':
	    state->sortFunc = vd_name_compare_func;
	    return;
	case 'p':
	    state->sortFunc = vd_pid_compare_func;
	    return;
	case 'c':
	    state->sortFunc = vd_cpu_compare_func;
	    return;
	case 'm':
	    state->sortFunc = vd_mem_compare_func;
	    return;
    	case 'q':
	    SHUTDOWN_FLAG = 1;
	    return;
    	default:
	    break;
    }
    
    if (!state->cmdBuffer)
    {
	state->cmdBuffer = ch;
	state->timeoutActive = 1;
	clock_gettime(CLOCK_REALTIME, &state->timeoutStart);
	    
	return;
    }
    else if (ch != state->cmdBuffer) 
    {
	state->cmdBuffer = '\0';
	state->timeoutActive = 0;
    
	return;
    }

    switch (ch)
    {
	case 'd':
	    state->cmdBuffer = '\0';
	    state->timeoutActive = 0;

	    kill(vd[state->selectedIndex]->pid, SIGTERM);

	    return;
	default:
	    break;
    }
}

void print_stats(
    ProcessListState *state,
    WindowData *wd,
    ProcessStatsViewData **vd,
    int count,
    Arena *procArena
)
{
    if (vd == NULL) return;
        
    const char *commandTitle = "Command";
    const char *pidTitle = "PID";
    const char *cpuTitle = "CPU %";
    const char *memTitle = "Memory %";
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
    
    SET_COLOR(win, MT_PAIR_BOX);
    
    werase(win);
    box(win, 0, 0);


#ifdef DEBUG
    char cmd = state->cmdBuffer ? state->cmdBuffer : '0';
    wattron(win, COLOR_PAIR(MT_PAIR_PRC_HEADER));
    mvwprintw(win, 
	windowTitleY, windowTitleX, 
	" 1st idx = %u, last = %u, selectedIndex = %u, toActive = %u, cmdBuf = %c ",
	state->firstIndexDisplayed, state->lastIndexDisplayed, state->selectedIndex,
	state->timeoutActive, state->cmdBuffer);
    wattroff(win, COLOR_PAIR(MT_PAIR_PRC_HEADER));
#else
    PRINTFC(win, windowTitleY, windowTitleX, " %s ", wd->windowTitle, MT_PAIR_PRC_HEADER);
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
    
    for (u8 i = 0; i < wd->wHeight - winDataOffset && i < count; i++)
    {
    	u16 idx = i + state->firstIndexDisplayed;
    	u8 isSelectedIndex = 
    		(state->selectedIndex - state->firstIndexDisplayed) + dataOffsetY == posY;
    
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

void adjust_state(ProcessListState *state, ProcessStats *stats)
{
    if (state->maxIndex == (s8)stats->count - 1) return;
    
    state->maxIndex = stats->count - 1;
    
    state->selectedIndex = state->selectedIndex > state->maxIndex ?
	state->maxIndex :
	state->selectedIndex;
    
    state->firstIndexDisplayed = state->selectedIndex > state->numOptsVisible - 1 ?
	state->maxIndex - state->numOptsVisible - 1 :
	0;
    state->lastIndexDisplayed = state->firstIndexDisplayed + state->numOptsVisible - 1;
}
