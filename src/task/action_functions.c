#include <assert.h>
#include <ncurses.h>
#include <stdlib.h>

#include "../../include/task.h"
#include "../../include/window.h"
#include "../../include/context_types.h"
#include "../../include/prc_list_util.h"
#include "../../include/menu.h"

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

	show_prc_info(*data, prcWin, winSelected);
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

    resize_win(ui);
    setup_list_state(context->listState, context->curPrcs, prcWin);
}

void refresh_action_fn(UIData *ui, void *ctx)
{
    u8 *context = (u8 *)ctx;
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *optWin = ui->windows[OPT_WIN];
    WindowData *statTypeWin = ui->windows[STAT_TYPE_WIN];

    if (ui->optionsVisible)
    {
	display_options(ui);
	wnoutrefresh(optWin->window);
    }
    else if (ui->menu->isVisible)
    {
	display_menu_options(ui);
	wnoutrefresh(statTypeWin->window);
    }

    // this is literally just so the compiler won't
    // complain. I know it kinda defeats the purpose
    // of some of the compiler flags, but this task
    // really didn't need any context data.
    context++;

    wnoutrefresh(container->window);
    doupdate();
}

void tg_cleanup(Arena *a)
{
    a_free(a);

    *a = a_new(64);
}
