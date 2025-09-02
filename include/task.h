#ifndef TASK_H
#define TASK_H

#include <arena.h>
#include <sys/sysinfo.h>

#include "thread_safe_queue.h"
#include "mt_type_defs.h"
#include "monitor.h"
#include "window.h"

#define BROKER_BUILD_TASK(tg, cond, data_fn, ...)	\
    do {											\
		if (cond)									\
		{											\
		    UITask *task = data_fn(__VA_ARGS__);	\
			UITask **link = (tg)->head ?			\
				&((tg)->tail->next) : &((tg)->head);\
													\
			*link = task;							\
			(tg)->tail = task;						\
		}											\
    } while(0)


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
    void (*cleanup)(Arena *a);
} TaskGroup;

/*
    
    task_broker.c

*/
void broker_init(ThreadSafeQueue *queue, size_t taskArenaCapacity);
void broker_commit(TaskGroup **tg);
TaskGroup * broker_create_group();
TaskGroup * broker_read();
void broker_cleanup();

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
UITask * build_uptime_load_average_task(Arena *taskArena);
UITask * build_print_time_task(Arena *taskArena);
UITask * build_print_header_task(Arena *taskArena);
UITask * build_print_footer_task(Arena *taskArena);

/*

    action_functions.c

*/
void cpu_action_fn(UIData *ui, void *ctx);
void mem_action_fn(UIData *ui, void *ctx);
void process_action_fn(UIData *ui, void *ctx);
void input_action_fn(UIData *ui, void *ctx);
void resize_action_fn(UIData *ui, void *ctx);
void refresh_action_fn(UIData *ui, void *ctx);
void print_uptime_loadavg_fn(UIData *ui, void *ctx);
void print_time_fn(UIData *ui, void *ctx);
void print_header_fn(UIData *ui, void *ctx);
void print_footer_fn(UIData *ui, void *ctx);

#endif
