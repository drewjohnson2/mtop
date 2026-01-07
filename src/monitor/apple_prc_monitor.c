#include <stdlib.h>
#include <unistd.h>
#include <libproc.h>
#include <sys/proc.h>
#include <mach/mach_time.h>

#include "../../include/monitor.h"

#define MAX_PROC_REGIONS_ALLOCD 3
#define TO_NS_DENOM 10000000

static char _get_state(struct proc_bsdinfo taskInfo);

ProcessesSummary * pm_get_processes(
    Arena *procArena,
    int (*sortFn)(const void *, const void *)
)
{
    pid_t pids[2048];
    uid_t uid = geteuid();
    u16 pidCount = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids)) / sizeof(pid_t);
    
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

    procStats->count = 0;

    // code
    for (size_t i = 0; i < (size_t)pidCount; i++)
    {
        if (procStats->count > MAX_PROCS - 1) break;

        Process **item = &procStats->processes[procStats->count];
        struct proc_taskallinfo taskInfo;

        proc_pidinfo(pids[i], PROC_PIDTASKALLINFO, 0, &taskInfo, sizeof(taskInfo));

        if (taskInfo.pbsd.pbi_uid != uid) continue;

        (*item) = a_alloc(procArena, sizeof(Process), __alignof(Process));

        strncpy((*item)->procName, taskInfo.pbsd.pbi_name, sizeof((*item)->procName) - 1);
        (*item)->ppid = taskInfo.pbsd.pbi_ppid;
        (*item)->pid = pids[i];
        (*item)->utime = taskInfo.ptinfo.pti_total_user;
        (*item)->stime = taskInfo.ptinfo.pti_total_system;
        (*item)->vmRss = taskInfo.ptinfo.pti_resident_size;
        (*item)->vmSize = taskInfo.ptinfo.pti_virtual_size;
        (*item)->threads = taskInfo.ptinfo.pti_threadnum;
        (*item)->state = _get_state(taskInfo.pbsd);
        (*item)->sysCallsMach = taskInfo.ptinfo.pti_syscalls_mach;
        (*item)->sysCallsUnix = taskInfo.ptinfo.pti_syscalls_unix;
        (*item)->priority = taskInfo.ptinfo.pti_priority;
        (*item)->faults = taskInfo.ptinfo.pti_faults;
        (*item)->messagesSent = taskInfo.ptinfo.pti_messages_sent;
        (*item)->messagesReceived = taskInfo.ptinfo.pti_messages_received;
        // (*item)->vmLock = 0;
        // (*item)->vmSwap = 0;
        // (*item)->vmData = 0;
        // (*item)->vmStack = 0;
        // (*item)->vmExe = 0;
        // (*item)->vmLib = 0;

        procStats->count++;
    }

    qsort(procStats->processes, procStats->count, sizeof(Process *), sortFn);

    procStats->cpuTimeAtSample = mach_absolute_time();

    return procStats;
}

void pm_get_info_by_pid(u32 pid, Process *prc)
{
    struct proc_taskallinfo taskInfo;

    proc_pidinfo(pid, PROC_PIDTASKALLINFO, 0, &taskInfo, sizeof(taskInfo));

    prc->threads = taskInfo.ptinfo.pti_threadnum;
    prc->ppid = taskInfo.pbsd.pbi_ppid;
    prc->vmRss = taskInfo.ptinfo.pti_resident_size;
    prc->vmSize = taskInfo.ptinfo.pti_virtual_size;
    prc->state = _get_state(taskInfo.pbsd);
    prc->sysCallsMach = taskInfo.ptinfo.pti_syscalls_mach;
    prc->sysCallsUnix = taskInfo.ptinfo.pti_syscalls_unix;
    prc->priority = taskInfo.ptinfo.pti_priority;
    prc->faults = taskInfo.ptinfo.pti_faults;
    prc->messagesSent = taskInfo.ptinfo.pti_messages_sent;
    prc->messagesReceived = taskInfo.ptinfo.pti_messages_received;
}

static char _get_state(struct proc_bsdinfo taskInfo)
{
    switch (taskInfo.pbi_status)
    {
        case SRUN: return 'R';
        case SIDL: return 'I';
        case SSLEEP: return 'S';
        case SSTOP: return 'T';
        case SZOMB: return 'Z';
        default: return '?';
    }

    return '?';
}
