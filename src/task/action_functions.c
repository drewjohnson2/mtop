#include "../../include/window.h"
#include "../../include/context_types.h"

void cpu_action_function(Arena *a, DisplayItems *di, void *ctx)
{
    CpuDataContext *context = (CpuDataContext *)ctx;
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
