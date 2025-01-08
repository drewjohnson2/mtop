#include <assert.h>

#include "../include/sorting.h"
#include "../include/monitor.h"
#include "../include/window.h"

typedef enum _sort_order 
{
    COMMAND
} SORT_ORDER;

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
