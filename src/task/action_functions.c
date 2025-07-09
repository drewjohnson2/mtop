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
