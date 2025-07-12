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

    task->action = cpu_action_fn;
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

    task->action = mem_action_fn;
    task->data = ctx;
    task->next = NULL;

    return task;
}

UITask * build_prc_task(
    Arena *taskArena,
    ProcessListState *listState,
    ProcessesSummary *prevPrcs,
    ProcessesSummary *curPrcs,
    ProcessInfoData *prcInfo,
    u64 memTotal
)
{
    UITask *task = a_alloc(taskArena, sizeof(UITask), __alignof(UITask));
    ProcessesContext *ctx = a_alloc(taskArena, sizeof(ProcessesContext), __alignof(ProcessesContext));

    ctx->listState = listState;
    ctx->prevPrcs = prevPrcs;
    ctx->curPrcs = curPrcs;
    ctx->memTotal = memTotal;
    ctx->processInfo = prcInfo;

    task->action = process_action_fn;
    task->data = ctx;
    task->next = NULL;

    return task;
}

UITask * build_input_task(
    Arena *taskArena,
    ProcessListState *listState
)
{
    UITask *task = a_alloc(taskArena, sizeof(UITask), __alignof(UITask));
    InputContext *ctx = a_alloc(taskArena, sizeof(InputContext), __alignof(InputContext));

    ctx->listState = listState;

    task->action = input_action_fn;
    task->data = ctx;
    task->next = NULL;

    return task;
}

UITask * build_resize_task(Arena *taskArena, ProcessListState *listState, ProcessesSummary *curPrcs)
{
    UITask *task = a_alloc(taskArena, sizeof(UITask), __alignof(UITask));
    ResizeContext *ctx = a_alloc(taskArena, sizeof(ResizeContext), __alignof(ResizeContext));

    ctx->curPrcs = curPrcs;
    ctx->listState = listState;

    task->action = resize_action_fn;
    task->data = ctx;
    task->next = NULL;

    return task;
}

	// if (RESIZE)
	// {
	//     resize_win(di); 
	//     _setup_list_state(listState, curPrcs, prcWin);
	// }

