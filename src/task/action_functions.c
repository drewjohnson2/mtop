#include <stdlib.h>

#include "../../include/task.h"
#include "../../include/window.h"
#include "../../include/context_types.h"

void cpu_action_function(DisplayItems *di, void *ctx)
{
    CpuDataContext *context = (CpuDataContext *)ctx;
    Arena *a = context->arena;
    GraphData *cpuGraphData = context->graphData;
    WindowData *cpuWin = di->windows[CPU_WIN];

    add_graph_point(a, cpuGraphData, context->cpuPercentage, 1);
    graph_render(
        a,
        cpuGraphData,
        cpuWin,
        MT_PAIR_CPU_GP,
        MT_PAIR_CPU_HEADER,
        1
    );
}

void mem_action_function(DisplayItems *di, void *ctx)
{
    MemoryDataContext *context = (MemoryDataContext *)ctx;
    Arena *a = context->arena;
    GraphData *memGraphData = context->graphData;
    WindowData *memWin = di->windows[MEMORY_WIN];

    add_graph_point(a, memGraphData, context->memPercentage, 1);
    graph_render(
        a,
        memGraphData,
        memWin,
        MT_PAIR_MEM_GP,
        MT_PAIR_MEM_HEADER,
        1
    );
}

void process_action_func(DisplayItems *di, void *ctx)
{
    ProcessesContext *context = (ProcessesContext *)ctx;
    ProcessStats *prevPrcs = context->prevPrcs;
    ProcessStats *curPrcs = context->curPrcs;
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
        listState->sortFunc
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

void input_action_func(DisplayItems *di, void *ctx)
{
    InputContext *context = (InputContext *)ctx;
    WindowData *container = di->windows[CONTAINER_WIN];
    ProcessListState *listState = context->listState;

    read_input(container->window, listState, di, NULL);
}
