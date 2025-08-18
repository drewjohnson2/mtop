#include <arena.h>
#include <bits/time.h>
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
#include "../../include/startup.h"
#include "../../include/text.h"

#define PRINT_TITLEFC(wd, y, x, fmt, val, pair) 		\
    do {							\
	if (*y >= wd->wHeight - 4) 				\
    	{							\
    	    *y = 4;						\
    	    *x = wd->wWidth / 2;				\
    	}							\
								\
	PRINTFC(wd->window, (*y)++, *x, fmt, val, pair);	\
    } while (0)

#define PRINT_VALUEFC(wd, y, x, fmt, val, padding, pair) 	\
    do {							\
	if (*y >= wd->wHeight - 4) 				\
    	{							\
    	    *y = 4;						\
    	    *x = wd->wWidth / 2;				\
    	}							\
								\
	u8 valuePos = padding + *x + 2;				\
								\
	PRINTFC(wd->window, (*y)++, valuePos, fmt, val, pair);	\
    } while (0)

void print_stats(
    ProcessListState *state,
    const WindowData *wd,
    ProcessStatsViewData **vd,
    s16 count,
    u8 winSelected
)
{
    if (vd == NULL || !mtopSettings->activeWindows[PRC_WIN]) return;
        
    const char *commandTitle = text(TXT_COMMAND);
    const char *pidTitle = text(TXT_PID);
    const char *cpuTitle = text(TXT_CPU);
    const char *memTitle = text(TXT_MEM);
    const u8 dataOffsetY = 4;
    const u16 dataOffsetX = 2;
    const u16 prcTblHeaderY = 2;
    const u16 windowTitleX = 3;
    const u16 windowTitleY = 0;
    const MT_Color_Pairs boxPair = winSelected ? MT_PAIR_SEL_WIN : MT_PAIR_BOX;
    
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
    SET_COLOR(win, boxPair);
    
    werase(win);
    box(win, 0, 0);


#ifdef DEBUG
    // May need this as a debug value again someday. Leaving for now.
    //char cmd = state->cmdBuffer ? state->cmdBuffer : '0'; 
    wattron(win, COLOR_PAIR(MT_PAIR_PRC_HEADER));
 //    mvwprintw(win, 
	// windowTitleY, windowTitleX, 
	// " 1st idx = %u, last = %u, selectedIndex = %u, maxidx = %u, toActive = %u, pc = %u, ap = %u",
	// state->pageStartIdx, state->pageEndIdx, state->selectedIndex,
	// state->count, state->timeoutActive, state->totalPages, state->activePage);
    mvwprintw(win, windowTitleY, windowTitleX, "Selected PID: %u", state->selectedPid);

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

	    for (size_t j = dataOffsetX; j < (size_t)(wd->wWidth - dataOffsetX); j++)
		PRINTFC(win, posY, j, "%c", ' ', pair);
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
    ProcessesSummary *curPrcs,
    ProcessesSummary *prevPrcs,
    u64 memTotal
)
{
    for (size_t i = 0; i < curPrcs->count; i++)
    {
	float cpuPct = 0.0;
	float memPct = 0.0;
	
	Process *target;
	Process *cur = curPrcs->processes[i];
	Process **match = bsearch(
	    &cur,
	    prevPrcs->processes,
	    prevPrcs->count,
	    sizeof(Process *),
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
	vd[i]->state = cur->state;
	vd[i]->ppid = cur->ppid;
	vd[i]->threads = cur->threads;
	vd[i]->vmRss = cur->vmRss;
	vd[i]->vmSize = cur->vmSize;
	vd[i]->vmLock = cur->vmLock;
	vd[i]->vmData = cur->vmData;
	vd[i]->vmStack = cur->vmStack;
	vd[i]->vmSwap = cur->vmSwap;
	vd[i]->vmExe = cur->vmExe;
	vd[i]->vmLib = cur->vmLib;
    }
}

void show_prc_info(ProcessStatsViewData *vd, const WindowData *wd, u8 winSelected) 
{
    Arena scratch = a_new(256);
    const u8 windowTitleY = 0;
    const u8 windowTitleX = 3;
    const u8 dataOffsetX = 2;
    const u8 valuePaddingLeft = 10;
    const MT_Color_Pairs boxPair = winSelected ? MT_PAIR_SEL_WIN : MT_PAIR_BOX;
    char prcInfoHeader[50];
    u8 posY = 2;
    u8 posX = 3;
    
    werase(wd->window);

    snprintf(
	prcInfoHeader,
	sizeof(prcInfoHeader),
	text(TXT_PRC_STAT_FMT),
	vd->pid,
	vd->command
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
    PRINTFC(wd->window, posY, posX, "%s", text(TXT_CPU_PCT_COL), MT_PAIR_PRC_STAT_NM);
    wattroff(wd->window, A_BOLD);
    PRINTFC(wd->window, posY++, valuePaddingLeft + posX + 2, "%.2f", vd->cpuPercentage,
	    MT_PAIR_PRC_STAT_VAL);

    wattron(wd->window, A_BOLD);
    PRINTFC(wd->window, posY, posX, "%s", text(TXT_MEM_PCT_COL), MT_PAIR_PRC_STAT_NM);
    wattroff(wd->window, A_BOLD);
    PRINTFC(wd->window, posY++, valuePaddingLeft + posX + 2, "%.2f", vd->memPercentage,
	    MT_PAIR_PRC_STAT_VAL);

    wattron(wd->window, A_BOLD);

    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_STATE), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_THREADS), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_PPID), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMRSS), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMSIZE), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMLOCK), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMDATA), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMSTACK), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMSWAP), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMEXE), MT_PAIR_PRC_STAT_NM);
    PRINT_TITLEFC(wd, &posY, &posX, "%s\t", text(TXT_VMLIB), MT_PAIR_PRC_STAT_NM);

    wattroff(wd->window, A_BOLD);

    posY = 6;

    PRINT_VALUEFC(wd, &posY, &posX, "%c", vd->state, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%d", vd->threads, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%hu", vd->ppid, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmRss, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmSize, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmLock, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmData, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmStack, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmSwap, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmExe, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINT_VALUEFC(wd, &posY, &posX, "%lu kB", vd->vmLib, valuePaddingLeft, MT_PAIR_PRC_STAT_VAL);
    PRINTFC(wd->window, wd->wHeight - 2, 3, "%s", text(TXT_RET_LIST_CTRL), MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 2, 5, "%s", text(TXT_RET_LIST), MT_PAIR_CTRL_TXT);
    SET_COLOR(wd->window, boxPair);

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
