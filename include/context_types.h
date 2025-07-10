#ifndef CONTEXT_TYPES_H
#define CONTEXT_TYPES_H

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
    ProcessStats *curPrcs;
    ProcessListState *listState;
    ProcessStatsViewData **vd;
} ProcessesContext;

typedef struct _input_context
{
    ProcessListState *listState;
    ProcessStatsViewData **vd;
} InputContext;

#endif
