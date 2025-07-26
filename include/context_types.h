#ifndef CONTEXT_TYPES_H
#define CONTEXT_TYPES_H

#include "monitor.h"
#include "window.h"

typedef struct _cpu_data_context 
{
    float cpuPercentage;
    GraphData *graphData;
    Arena *arena;
} CpuDataContext;

typedef struct _mem_data_context
{
    float memPercentage;
    GraphData *graphData;
    Arena *arena;
} MemoryDataContext;

typedef struct _prc_context
{
    u64 memTotal;
    ProcessesSummary *prevPrcs;
    ProcessesSummary *curPrcs;
    ProcessListState *listState;
} ProcessesContext;

typedef struct _input_context
{
    ProcessListState *listState;
} InputContext;

typedef struct _resize_context
{
    ProcessListState *listState;
    ProcessesSummary *curPrcs;
} ResizeContext;

#endif
