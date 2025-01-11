#include <assert.h>
#include <math.h>

#include "../include/sorting.h"
#include "../include/monitor.h"
#include "../include/window.h"

int prc_name_compare(const void *a, const void *b)
{
    assert(a && b);
    
    const ProcessList *x = *(ProcessList **)a;
    const ProcessList *y = *(ProcessList **)b;
    
    return strcmp(x->procName, y->procName);
}

int prc_pid_compare(const void *a, const void *b)
{
    assert(a && b);
    
    const ProcessList *x = *(ProcessList **)a;
    const ProcessList *y = *(ProcessList **)b;
    
    return x->pid - y->pid;
}

int vd_name_compare_func(const void *a, const void *b)
{
    assert(a && b);
    
    const ProcessStatsViewData *x = *(ProcessStatsViewData **)a;
    const ProcessStatsViewData *y = *(ProcessStatsViewData **)b;
    
    return strcmp(x->command, y->command);
}

int vd_pid_compare_func(const void *a, const void *b)
{
    assert(a && b);
    
    const ProcessStatsViewData *x = *(ProcessStatsViewData **)a;
    const ProcessStatsViewData *y = *(ProcessStatsViewData **)b;

    return x->pid - y->pid;
}

int vd_cpu_compare_func(const void *a, const void *b)
{
    assert(a && b);
    
    const ProcessStatsViewData *x = *(ProcessStatsViewData **)a;
    const ProcessStatsViewData *y = *(ProcessStatsViewData **)b;

    float diff = y->cpuPercentage - x->cpuPercentage;

    if (fabs(diff) < EPSILON) return 0;

    return (diff > 0) ? 1 : -1;
}

int vd_mem_compare_func(const void *a, const void *b)
{
    assert(a && b);
    
    const ProcessStatsViewData *x = *(ProcessStatsViewData **)a;
    const ProcessStatsViewData *y = *(ProcessStatsViewData **)b;

    float diff = y->memPercentage - x->memPercentage;

    if (fabs(diff) < EPSILON) return 0;

    return (diff > 0) ? 1 : -1;
}
