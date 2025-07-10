#ifndef TASK_H
#define TASK_H

#include <arena.h>

#include "mt_type_defs.h"
#include "monitor.h"
#include "window.h"

typedef struct _ui_task
{
    void (*action)(DisplayItems *di, void *ctx);
    void *data;
    void *next;
} UITask;

typedef struct _task_group
{
    UITask *tasks;
    u8 tasksComplete;
    void (*cleanup)();
} TaskGroup;

/*
    
    task_data_builder.c

*/
void init_data(Arena *cpuGraphArena, Arena *memGraphArena);
UITask * build_cpu_task(Arena *taskArena, Arena *actionArena, CpuStats *curStats, CpuStats *prevStats);
UITask * build_mem_task(Arena *taskArena, Arena *actionArena, MemoryStats *memStats);
UITask  *build_prc_task(
    Arena *taskArena,
    ProcessListState *listState,
    ProcessStats *prevPrcs,
    ProcessStats *curPrcs,
    ProcessInfoData *prcInfo,
    u64 memTotal
);
UITask * build_input_task(
    Arena *taskArena,
    ProcessListState *listState
);

/*

    task_functions.c

*/
void cpu_action_function(DisplayItems *di, void *ctx);
void mem_action_function(DisplayItems *di, void *ctx);
void process_action_func(DisplayItems *di, void *ctx);
void input_action_func(DisplayItems *di, void *ctx);
#endif
