#include <assert.h>
#include <ncurses.h>
#include <stdlib.h>

#include "../../include/task.h"
#include "../../include/window.h"
#include "../../include/context_types.h"
#include "../../include/prc_list_util.h"
#include "../../include/menu.h"
#include "../../include/text.h"
#include "../../include/input.h"

void cpu_action_fn(UIData *ui, void *ctx)
{
    CpuDataContext *context = (CpuDataContext *)ctx;
    Arena *a = context->arena;
    WindowData *cpuWin = ui->windows[CPU_WIN];

    u8 winSelected = ui->mode == ARRANGE && ui->selectedWindow == CPU_WIN;

    add_graph_point(a, context->graphData, context->cpuPercentage, 1);
    graph_render(
        a,
        context->graphData,
        cpuWin,
        MT_PAIR_CPU_GP,
        MT_PAIR_CPU_HEADER,
		winSelected
    );

    wnoutrefresh(cpuWin->window);
}

void mem_action_fn(UIData *ui, void *ctx)
{
    MemoryDataContext *context = (MemoryDataContext *)ctx;
    Arena *a = context->arena;
    WindowData *memWin = ui->windows[MEMORY_WIN];
    u8 winSelected = ui->mode == ARRANGE && ui->selectedWindow == MEMORY_WIN;

    add_graph_point(a, context->graphData, context->memPercentage, 1);
    graph_render(
        a,
		context->graphData,
        memWin,
        MT_PAIR_MEM_GP,
        MT_PAIR_MEM_HEADER,
		winSelected
    );

    wnoutrefresh(memWin->window);
}

void process_action_fn(UIData *ui, void *ctx)
{
    ProcessesContext *context = (ProcessesContext *)ctx;
    ProcessesSummary *prevPrcs = context->prevPrcs;
    ProcessesSummary *curPrcs = context->curPrcs;
    ProcessListState *listState = context->listState;
    WindowData *prcWin = ui->windows[PRC_WIN];
    u64 memTotal = context->memTotal;
    u8 winSelected = ui->mode == ARRANGE && ui->selectedWindow == PRC_WIN;

    if (ui->reinitListState)
    {
		plu_setup_list_state(listState, curPrcs, prcWin);
		ui->reinitListState = false;
    }

    Arena scratch = a_new(
		(sizeof(ProcessStatsViewData *) * curPrcs->count) +
    	sizeof(ProcessStatsViewData) +
    	(sizeof(ProcessStatsViewData) * curPrcs->count)
    );
    ProcessStatsViewData **vd = a_alloc(
		&scratch,
    	sizeof(ProcessStatsViewData *) * curPrcs->count,
    	__alignof(ProcessStatsViewData *)
    ); 
    
    set_prc_view_data(
		&scratch,
    	vd,
    	curPrcs,
    	prevPrcs,
    	memTotal
    );		
    qsort(
        vd,
        curPrcs->count,
        sizeof(ProcessStatsViewData *),
        listState->sortFn
    );

    if (!listState->infoVisible)
    {
		listState->selectedPid = vd[listState->selectedIndex]->pid;
		print_stats(listState, prcWin, vd, curPrcs->count, winSelected);
    }
    else
    {	
		qsort(vd, curPrcs->count, sizeof(ProcessStatsViewData *), vd_pid_compare_without_direction_fn);

		ProcessStatsViewData **data = bsearch(
		    &listState->selectedPid,
		    vd,
		    curPrcs->count,
		    sizeof(ProcessStatsViewData *),
		    vd_find_by_pid_compare_fn
		);

        if (data) show_prc_info(*data, prcWin, winSelected);
        else listState->infoVisible = false;
    }

    wnoutrefresh(prcWin->window);
    a_free(&scratch);
}

void input_action_fn(UIData *ui, void *ctx)
{
    InputContext *context = (InputContext *)ctx;
    WindowData *container = ui->windows[CONTAINER_WIN];

    if (ui->mode == NORMAL) read_normal_input(container->window, context->listState, ui);
    else read_arrange_input(ui);
}

void resize_action_fn(UIData *ui, void *ctx)
{
    ResizeContext *context = (ResizeContext *)ctx;
    WindowData *prcWin = ui->windows[PRC_WIN];

    resize_windows(ui);
    plu_setup_list_state(context->listState, context->curPrcs, prcWin);
}

void refresh_action_fn(UIData *ui, void *ctx)
{
    u8 *context = (u8 *)ctx;
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *optWin = ui->windows[OPT_WIN];
    WindowData *menuWin = ui->windows[MENU_WIN];
	void (*optionFn)(UIData *) = ui->mode == NORMAL ? display_normal_options : display_arrange_options;

    if (ui->optionsVisible)
    {
		optionFn(ui);
		wnoutrefresh(optWin->window);
    }
    else if (ui->menu->isVisible)
    {
		menu_display_options(ui);
		wnoutrefresh(menuWin->window);
    }

    // this is literally just so the compiler won't
    // complain. I know it kinda defeats the purpose
    // of some of the compiler flags, but this task
    // really didn't need any context data.
    context++;

    wnoutrefresh(container->window);
    doupdate();
}

void print_uptime_loadavg_fn(UIData *ui, void *ctx)
{
    // move platform specific code to their own functions
#if defined (__linux__)
	LoadUptimeContext *context = (LoadUptimeContext *)ctx;
	struct sysinfo info = context->info;
	const WindowData *container = ui->windows[CONTAINER_WIN];
    u16 days;
    u64 uptime;
    u16 hours;
    u16 minutes;
    u16 seconds;
    char displayStr[66];

    uptime = info.uptime;
    days = uptime / 86400;
    uptime %= 86400;
    hours = uptime / 3600;
    uptime %= 3600;
    minutes = uptime / 60;
    seconds = uptime %= 60;

    snprintf(
		displayStr,
		sizeof(displayStr),
		text(TXT_UPTIME_LOAD_FMT),
		days,
		hours,
		minutes,
		seconds,
		context->load[0],
		context->load[1],
		context->load[2]
    );

    if (strlen(displayStr) > (container->wWidth / 2)) return;

    const u8 uptimeX = (container->wWidth / 2) - (strlen(displayStr) / 2);

    PRINTFC(container->window, 0, uptimeX, "%s", displayStr, MT_PAIR_TM);
#endif
}

void print_time_fn(UIData *ui, void *ctx)
{
	PrintTimeContext *context = (PrintTimeContext *)ctx;
    char timeBuf[10];
	const WindowData *container = ui->windows[CONTAINER_WIN];

    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &context->tmNow);

    PRINTFC(container->window, 0, container->wWidth - 10, "%s", timeBuf, MT_PAIR_TM);
}

void print_header_fn(UIData *ui, void *ctx)
{
	char *user = (char *)ctx;
	WindowData *container = ui->windows[CONTAINER_WIN];

    wattron(container->window, A_BOLD);
    PRINTFC(container->window, 0, 2, "%s", text(TXT_MTOP), MT_PAIR_MTOP_LBL);
    wattroff(container->window, A_BOLD);
    PRINTFC(container->window, 0, 7, "for %s", user, MT_PAIR_USR_LBL);
}

void print_footer_fn(UIData *ui, void *ctx)
{
	WindowData *container = ui->windows[CONTAINER_WIN];
    u8 *context = (u8 *)ctx;

	context++;

    const u8 githubText = container->wWidth - 30;

    if (!mtopSettings->activeWindows[PRC_WIN])
    {
		PRINTFC(container->window, container->wHeight - 1, githubText, "%s", 
		    text(TXT_GITHUB), MT_PAIR_GITHUB);

		return;
    }

    const u8 killPrcCtrlX = 2;
    const u8 killPrcLabelX = killPrcCtrlX + 2;
    const u8 downCtrlX = 19;
    const u8 downLableX = downCtrlX + 2;
    const u8 upCtrlX = 27;
    const u8 upLabelX = upCtrlX + 2;
    const u8 pLeftCtrlX = 33;
    const u8 pLeftLabelX = pLeftCtrlX + 2;
    const u8 pRightCtrlX = 46;
    const u8 pRightLabelX = pRightCtrlX + 2;
    const u8 optionsCtrlX = 60;
    const u8 optionsLabelX = optionsCtrlX + 2;

    PRINTFC(container->window, container->wHeight - 1, killPrcCtrlX, "%s", text(TXT_KILL_CTRL), MT_PAIR_CTRL);
    PRINTFC(container->window, container->wHeight - 1, killPrcLabelX, " %s ",
	    text(TXT_KILL), MT_PAIR_CTRL_TXT);
    PRINTFC(container->window, container->wHeight - 1, downCtrlX, "%s", text(TXT_DOWN_CTRL), MT_PAIR_CTRL);
    PRINTFC(container->window, container->wHeight - 1, downLableX, "%s", text(TXT_DOWN), MT_PAIR_CTRL_TXT);
    PRINTFC(container->window, container->wHeight - 1, upCtrlX, "%s", text(TXT_UP_CTRL), MT_PAIR_CTRL);
    PRINTFC(container->window, container->wHeight - 1, upLabelX, "%s", text(TXT_UP), MT_PAIR_CTRL_TXT);
    PRINTFC(container->window, container->wHeight - 1, pLeftCtrlX, "%s", text(TXT_PAGE_LEFT_CTRL), MT_PAIR_CTRL);
    PRINTFC(container->window, container->wHeight - 1, pLeftLabelX, "%s", text(TXT_PAGE_LEFT), MT_PAIR_CTRL_TXT);
    PRINTFC(container->window, container->wHeight - 1, pRightCtrlX, "%s", text(TXT_PAGE_RIGHT_CTRL), MT_PAIR_CTRL);
    PRINTFC(container->window, container->wHeight - 1, pRightLabelX, "%s", text(TXT_PAGE_RIGHT), MT_PAIR_CTRL_TXT);
    PRINTFC(container->window, container->wHeight - 1, optionsCtrlX, "%s", text(TXT_OPT_CTRL), MT_PAIR_CTRL);
    PRINTFC(container->window, container->wHeight - 1, optionsLabelX, "%s", text(TXT_OPT), MT_PAIR_CTRL_TXT);
    PRINTFC(container->window, container->wHeight - 1, githubText, "%s", text(TXT_GITHUB), MT_PAIR_GITHUB);
}
