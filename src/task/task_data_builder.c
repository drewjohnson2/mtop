#include <arena.h>

#include "../../include/monitor.h"
#include "../../include/task.h"
#include "../../include/context_types.h"

GraphData *cpuGraphData = NULL;
GraphData *memGraphData = NULL;
ProcessListState *listState = NULL;

void init_data(Arena *cpuGraphArena, Arena *memGraphArena)
{
    cpuGraphData = a_alloc(cpuGraphArena, sizeof(GraphData), __alignof(GraphData));
    memGraphData = a_alloc(memGraphArena, sizeof(GraphData), __alignof(GraphData));
}

UITask * build_cpu_task(Arena *taskArena, Arena *actionArena, CpuStats *curStats, CpuStats *prevStats)
{
    float cpuPercentage;
    UITask *task = a_alloc(taskArena, sizeof(UITask), __alignof(UITask));

    CALCULATE_CPU_PERCENTAGE(prevStats, curStats, cpuPercentage);

    CpuDataContext *ctx = a_alloc(taskArena, sizeof(CpuDataContext), __alignof(CpuDataContext));

    ctx->cpuPercentage = cpuPercentage;
    ctx->graphData = cpuGraphData;
    ctx->arena = actionArena;

    task->action = cpu_action_function;
    task->data = ctx;
    task->next = NULL;

    return task;
}

UITask * build_mem_task(Arena *taskArena, Arena *actionArena, MemoryStats *memStats)
{
    float memoryPercentage;
    UITask *task = a_alloc(taskArena, sizeof(UITask), __alignof(UITask));

    CALCULATE_MEMORY_USAGE(memStats, memoryPercentage);

    MemoryDataContext *ctx = a_alloc(taskArena, sizeof(MemoryDataContext), __alignof(MemoryDataContext));

    ctx->memPercentage = memoryPercentage;
    ctx->graphData = memGraphData;
    ctx->arena = actionArena;

    task->action = mem_action_function;
    task->data = ctx;
    task->next = NULL;

    return task;
}

UITask * build_prc_task(
    Arena *taskArena,
    ProcessListState *listState,
    ProcessStatsViewData **vd,
    ProcessStats *curPrcs
)
{
    UITask *task = a_alloc(taskArena, sizeof(UITask), __alignof(UITask));
    ProcessesContext *ctx = a_alloc(taskArena, sizeof(ProcessesContext), __alignof(ProcessesContext));

    ctx->listState = listState;
    ctx->curPrcs = curPrcs;
    ctx->vd = vd;

    task->action = process_action_func;
    task->data = ctx;
    task->next = NULL;

    return task;
}

UITask * build_input_task(
    Arena *taskArena,
    ProcessListState *listState,
    ProcessStatsViewData **vd
)
{
    UITask *task = a_alloc(taskArena, sizeof(UITask), __alignof(UITask));
    InputContext *ctx = a_alloc(taskArena, sizeof(InputContext), __alignof(InputContext));

    ctx->listState = listState;
    ctx->vd = vd;

    task->action = input_action_func;
    task->data = ctx;
    task->next = NULL;

    return task;
}
