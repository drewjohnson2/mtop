#include <arena.h>

#if defined (__linux__)
#include <proc/readproc.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/monitor.h"

#define MAX_PROC_REGIONS_ALLOCD 3

ProcessesSummary * pm_get_processes(
    Arena *procArena,
    int (*sortFn)(const void *, const void *)
) 
{
#if defined (__linux__)
    uid_t uid = getuid();
    
    if (procArena->regionsAllocated > MAX_PROC_REGIONS_ALLOCD)
    {
		r_free_head(procArena);
    }
    
    ProcessesSummary *procStats = a_alloc(
		procArena,
    	sizeof(ProcessesSummary),
    	__alignof(ProcessesSummary)
    );
    procStats->processes = a_alloc(
		procArena,
		sizeof(Process *) * MAX_PROCS,
		__alignof(Process *)
    );
    
    PROCTAB *prc = openproc(PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLCOM);
    proc_t *prcInfo;

    while ((prcInfo = readproc(prc, NULL)) != NULL)
    {
		if (procStats->count > MAX_PROCS - 1) break;
		if (prcInfo->ruid != (int)uid)
		{
		    freeproc(prcInfo);
		    continue;
		}

		Process **item = &procStats->processes[procStats->count];

		(*item) = a_alloc(procArena, sizeof(Process), __alignof(Process));

		strcpy((*item)->procName, prcInfo->cmd);
		(*item)->pid = prcInfo->tid;
		(*item)->stime = prcInfo->stime;
		(*item)->utime = prcInfo->utime;
		(*item)->vmRss = prcInfo->vm_rss;
		(*item)->vmSize = prcInfo->vm_size;
		(*item)->vmLock = prcInfo->vm_lock;
		(*item)->vmData = prcInfo->vm_data;
		(*item)->vmStack = prcInfo->vm_stack;
		(*item)->vmSwap = prcInfo->vm_swap;
		(*item)->vmExe = prcInfo->vm_exe;
		(*item)->vmLib = prcInfo->vm_lib;
		(*item)->threads = prcInfo->nlwp;
		(*item)->ppid = prcInfo->ppid;
		(*item)->state = prcInfo->state;
		procStats->count++;

		freeproc(prcInfo);
    }

    closeproc(prc);
    
    procStats->cpuTimeAtSample = cpu_time_now();
    
    qsort(procStats->processes, procStats->count, sizeof(Process *), sortFn);
    
    return procStats;
#endif
}


void pm_get_info_by_pid(u32 pid, Process *prc)
{
#if defined (__linux__)
    PROCTAB *proc = openproc(PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLCOM);

    if (!proc) return;

    proc_t *out;

    while ((out = readproc(proc, NULL)) != NULL) 
    {
		if (out->tid == (int)pid)
		{
		    prc->vmRss = out->vm_rss;
		    prc->vmSize = out->vm_size;
		    prc->vmLock = out->vm_lock;
		    prc->vmData = out->vm_data;
		    prc->vmStack = out->vm_stack;
		    prc->vmSwap = out->vm_swap;
		    prc->vmExe = out->vm_exe;
		    prc->vmLib = out->vm_lib;
		    prc->threads = out->nlwp;
		    prc->ppid = out->ppid;
		    prc->state = out->state;

		    freeproc(out);
		    closeproc(proc);

		    return;
		}

		freeproc(out);
    }

    closeproc(proc);
#endif
}
