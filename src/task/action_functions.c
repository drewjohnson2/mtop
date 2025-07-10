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
    ProcessStats *curPrcs = context->curPrcs;
    ProcessListState *listState = context->listState;
    ProcessStatsViewData **vd = context->vd;
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *prcWin = di->windows[PRC_WIN];

    // This needs to change. Investigate how it is that we can get here
    // but vd is null.
    if (vd)
    {
	listState->selectedPid = vd[listState->selectedIndex]->pid;
	qsort(vd, curPrcs->count, sizeof(ProcessStatsViewData *), listState->sortFunc);
    }

    if (!listState->infoVisible)
	print_stats(listState, prcWin, vd, curPrcs->count);
}

void input_action_func(DisplayItems *di, void *ctx)
{
    InputContext *context = (InputContext *)ctx;
    WindowData *container = di->windows[CONTAINER_WIN];
    ProcessListState *listState = context->listState;
    ProcessStatsViewData **vd = context->vd;

    read_input(container->window, listState, di, vd, NULL);
}
