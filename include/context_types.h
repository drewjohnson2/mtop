#ifndef CONTEXT_TYPES_H
#define CONTEXT_TYPES_H

#if defined (__linux__)
#include <sys/sysinfo.h>
#endif

#include "monitor.h"
#include "window.h"

typedef struct 
{
    float cpuPercentage;
    GraphData *graphData;
    Arena *arena;
} CpuDataContext;

typedef struct 
{
    float memPercentage;
    GraphData *graphData;
    Arena *arena;
} MemoryDataContext;

typedef struct 
{
    u64 memTotal;
    ProcessesSummary *prevPrcs;
    ProcessesSummary *curPrcs;
    ProcessListState *listState;
} ProcessesContext;

typedef struct 
{
    ProcessListState *listState;
} InputContext;

typedef struct
{
    ProcessListState *listState;
    ProcessesSummary *curPrcs;
} ResizeContext;

typedef struct
{
	double load[3];
    time_t uptime;
} LoadUptimeContext;

typedef struct
{
	struct tm tmNow;
} PrintTimeContext;

#endif
