#ifndef TASK_H
#define TASK_H

#include <arena.h>

#include "mt_type_defs.h"
#include "monitor.h"
#include "window.h"

typedef struct _ui_task
{
    void (*action)(UIData *ui, void *ctx);
    void *data;
    struct _ui_task *next;
} UITask;

typedef struct
{
    Arena a;
    UITask *head;
    UITask *tail;
    u8 tasksComplete;
    void (*cleanup)(Arena *a);
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
    ProcessesSummary *prevPrcs,
    ProcessesSummary *curPrcs,
    u64 memTotal
);
UITask * build_input_task(
    Arena *taskArena,
    ProcessListState *listState
);
UITask * build_resize_task(Arena *taskArena, ProcessListState *listState, ProcessesSummary *curPrcs);
UITask * build_refresh_task(Arena *taskArena);
/*

    task_functions.c

*/
void cpu_action_fn(UIData *ui, void *ctx);
void mem_action_fn(UIData *ui, void *ctx);
void process_action_fn(UIData *ui, void *ctx);
void input_action_fn(UIData *ui, void *ctx);
void resize_action_fn(UIData *ui, void *ctx);
void refresh_action_fn(UIData *ui, void *ctx);
void tg_cleanup(Arena *a);

#endif
