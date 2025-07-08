#include <arena.h>

#include "../../include/monitor.h"
#include "../../include/task.h"
#include "../../include/context_types.h"

GraphData *cpuGraphData = NULL;

void init_data(Arena *cpuGraphArena)
{
    cpuGraphData = a_alloc(cpuGraphArena, sizeof(GraphData), __alignof(GraphData));
}

UITask * build_cpu_task(Arena *taskArena, CpuStats *curStats, CpuStats *prevStats)
{
    float cpuPercentage;
    UITask *task = a_alloc(taskArena, sizeof(UITask), __alignof(UITask));

    CALCULATE_CPU_PERCENTAGE(prevStats, curStats, cpuPercentage);

    CpuDataContext *ctx = a_alloc(taskArena, sizeof(CpuDataContext), __alignof(CpuDataContext));

    ctx->cpuPercentage = cpuPercentage;
    ctx->graphData = cpuGraphData;

    task->action = cpu_action_function;
    task->data = ctx;

    return task;
}
