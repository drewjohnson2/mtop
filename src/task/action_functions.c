#include <stdlib.h>

#include "../../include/task.h"
#include "../../include/window.h"
#include "../../include/context_types.h"
#include "../../include/prc_list_util.h"

void cpu_action_fn(DisplayItems *di, void *ctx)
{
    CpuDataContext *context = (CpuDataContext *)ctx;
    Arena *a = context->arena;
    WindowData *cpuWin = di->windows[CPU_WIN];

    add_graph_point(a, context->graphData, context->cpuPercentage, 1);
    graph_render(
        a,
        context->graphData,
        cpuWin,
        MT_PAIR_CPU_GP,
        MT_PAIR_CPU_HEADER
    );
}

void mem_action_fn(DisplayItems *di, void *ctx)
{
    MemoryDataContext *context = (MemoryDataContext *)ctx;
    Arena *a = context->arena;
    WindowData *memWin = di->windows[MEMORY_WIN];

    add_graph_point(a, context->graphData, context->memPercentage, 1);
    graph_render(
        a,
	context->graphData,
        memWin,
        MT_PAIR_MEM_GP,
        MT_PAIR_MEM_HEADER
    );
}

void process_action_fn(DisplayItems *di, void *ctx)
{
    ProcessesContext *context = (ProcessesContext *)ctx;
    ProcessesSummary *prevPrcs = context->prevPrcs;
    ProcessesSummary *curPrcs = context->curPrcs;
    ProcessListState *listState = context->listState;
    ProcessInfoData *prcInfo = context->processInfo;
    WindowData *prcWin = di->windows[PRC_WIN];
    u64 memTotal = context->memTotal;

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
	print_stats(listState, prcWin, vd, curPrcs->count);
    }
    else
	show_prc_info(vd[listState->selectedIndex], prcInfo->info, prcWin);

    a_free(&scratch);
}

void input_action_fn(DisplayItems *di, void *ctx)
{
    InputContext *context = (InputContext *)ctx;
    WindowData *container = di->windows[CONTAINER_WIN];

    read_input(container->window, context->listState, di);
}

void resize_action_fn(DisplayItems *di, void *ctx)
{
    ResizeContext *context = (ResizeContext *)ctx;
    WindowData *prcWin = di->windows[PRC_WIN];

    resize_win(di);
    setup_list_state(context->listState, context->curPrcs, prcWin);
}

void tg_cleanup(Arena *a)
{
    a_free(a);

    *a = a_new(64);
}
