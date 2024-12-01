#include <bits/time.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "../include/window.h"
#include "../include/mt_colors.h"
#include "../include/monitor.h"
#include "../include/ui_utils.h"
#include "../include/thread.h"

void read_input(WINDOW *win, ProcessListState *state)
{
	char ch = wgetch(win);
	u8 executeCmd;

	if (ch == -1) return;

	if (!state->cmdBuffer)
	{
		state->cmdBuffer = ch;
		clock_gettime(CLOCK_REALTIME, &state->timeoutStart);

		return;
	}

	clock_gettime(CLOCK_REALTIME, &state->timeoutCurrent);

	u16 timeElapsedSec = state->timeoutCurrent.tv_sec - state->timeoutStart.tv_sec;
	u64 timeElapsedMs = (state->timeoutCurrent.tv_nsec - state->timeoutStart.tv_nsec) 
		/ 1000000;

	executeCmd = (ch == state->cmdBuffer) && 
		(timeElapsedSec <= 0) &&
		(timeElapsedMs < INPUT_TIMEOUT_MS);

	if (!executeCmd) 
	{
		state->cmdBuffer = '\0';
		state->timeoutStart.tv_nsec = 0;
		state->timeoutCurrent.tv_nsec = 0;

		return;
	}
	
	switch (ch)
	{
		case 'q':
			SHUTDOWN_FLAG = 1;
			return;
		default:
			return;
	}
}

int _vd_name_compare_func(const void *a, const void *b)
{
	assert(a && b);

	const ProcessStatsViewData *x = *(ProcessStatsViewData **)a;
	const ProcessStatsViewData *y = *(ProcessStatsViewData **)b;

	return strcmp(x->command, y->command);
}

void print_stats(WindowData *wd, ProcessStatsViewData **vd, int count, Arena *procArena)
{
	if (vd == NULL) return;

	qsort(vd, count, sizeof(ProcessStatsViewData *), _vd_name_compare_func);

	const char *commandTitle = "Command";
	const char *pidTitle = "PID";
	const char *cpuTitle = "CPU %";
	const char *memTitle = "Memory %";
	const u16 dataStartX = 2;
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

	SET_COLOR(win, MT_PAIR_PRC_HEADER);
#ifdef DEBUG
	mvwprintw(win, windowTitleY, windowTitleX, 
		   " Arena Regions Alloc'd = %zu ", procArena->regionsAllocated);
#else
	mvwprintw(win, windowTitleY, windowTitleX, " %s ", wd->windowTitle);
#endif

	SET_COLOR(win, MT_PAIR_PRC_TBL_HEADER);
	wattron(win, A_BOLD);

	mvwprintw(win, prcTblHeaderY, dataStartX, "%s", commandTitle);
	mvwprintw(win, prcTblHeaderY, pidPosX, "%s", pidTitle);

	if (fitCpu) mvwprintw(win, prcTblHeaderY, cpuPosX, "%s", cpuTitle);
	if (fitMem) mvwprintw(win, prcTblHeaderY, memPosX, "%s", memTitle);

	for (size_t x = 2; x < (size_t)wd->wWidth - 3; x++)
	{
		mvwprintw(win, prcTblHeaderY + 1, x, "%c", '-');
	}

	wattroff(win, A_BOLD);

	u8 i = 0;
	u8 posY = 4;

	while (i < wd->wHeight - 5 && i < count)
	{
		SET_COLOR(win, MT_PAIR_PRC_UNSEL_TEXT);

		mvwprintw(win, posY, dataStartX, "%s", vd[i]->command);
		mvwprintw(win, posY, pidPosX, "%d", vd[i]->pid);

		if (fitCpu)
		{
			if (vd[i]->cpuPercentage < 0.01) 
				SET_COLOR(win, MT_PAIR_PRC_PCT_ZERO);

			mvwprintw(win, posY, cpuPosX, "%.2f", vd[i]->cpuPercentage);
		}

		SET_COLOR(win, MT_PAIR_PRC_UNSEL_TEXT);

		if (fitMem) mvwprintw(win, posY++, memPosX, "%.2f", vd[i]->memPercentage);

		i++;
	}

	UNSET_COLOR(win, MT_PAIR_PRC_UNSEL_TEXT);
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
