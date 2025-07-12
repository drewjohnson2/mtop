#include <assert.h>
#include <math.h>
#include <string.h>

#include "../../include/sorting.h"
#include "../../include/monitor.h"
#include "../../include/window.h"

SortDirection sortDirection;

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

int prc_tracked_stat_cmp(const void *a, const void *b)
{
    assert(a && b);

    const char *x = (const char *)a;
    const char *y = *(const char **)b;

    return strncmp(x, y, strlen(y));
}

int vd_name_compare_fn(const void *a, const void *b)
{
    assert(a && b);
    
    const ProcessStatsViewData *x = *(ProcessStatsViewData **)a;
    const ProcessStatsViewData *y = *(ProcessStatsViewData **)b;
    
    int cmp = sortDirection == ASC ?
	strcmp(x->command, y->command) :
	strcmp(y->command, x->command);

    return cmp;
}

int vd_pid_compare_fn(const void *a, const void *b)
{
    assert(a && b);
    
    const ProcessStatsViewData *x = *(ProcessStatsViewData **)a;
    const ProcessStatsViewData *y = *(ProcessStatsViewData **)b;

    int cmp = sortDirection == ASC ?
	x->pid - y->pid :
	y->pid - x->pid;

    return cmp;
}

int vd_cpu_compare_fn(const void *a, const void *b)
{
    assert(a && b);
    
    const ProcessStatsViewData *x = *(ProcessStatsViewData **)a;
    const ProcessStatsViewData *y = *(ProcessStatsViewData **)b;

    float diff = y->cpuPercentage - x->cpuPercentage;
    s8 descRet = (diff > 0) ? 1 : -1;
    s8 ascRet = (diff > 0) ? -1 : 1;

    if (fabs(diff) < EPSILON) return 0;

    return sortDirection == DESC ? descRet : ascRet;
}

int vd_mem_compare_fn(const void *a, const void *b)
{
    assert(a && b);
    
    const ProcessStatsViewData *x = *(ProcessStatsViewData **)a;
    const ProcessStatsViewData *y = *(ProcessStatsViewData **)b;

    float diff = y->memPercentage - x->memPercentage;
    s8 descRet = (diff > 0) ? 1 : -1;
    s8 ascRet = (diff > 0) ? -1 : 1;

    if (fabs(diff) < EPSILON) return 0;

    return sortDirection == DESC ? descRet : ascRet;
}

