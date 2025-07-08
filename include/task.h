#ifndef TASK_H
#define TASK_H

#include <arena.h>

#include "mt_type_defs.h"
#include "monitor.h"
#include "window.h"

typedef struct _ui_task
{
    void (*action)(Arena *a, DisplayItems *di, void *ctx);
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
void init_data(Arena *cpuGraphArena);
UITask * build_cpu_task(Arena *taskArena, CpuStats *curStats, CpuStats *prevStats);

/*

    task_functions.c

*/
void cpu_action_function(Arena *a, DisplayItems *di, void *ctx);
#endif
