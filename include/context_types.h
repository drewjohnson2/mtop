#ifndef CONTEXT_TYPES_H
#define CONTEXT_TYPES_H

#include <sys/sysinfo.h>

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
	struct sysinfo info;
} LoadUptimeContext;

typedef struct
{
	struct tm tmNow;
} PrintTimeContext;

typedef struct
{
	char *user;
} PrintHeaderContext;

#endif
