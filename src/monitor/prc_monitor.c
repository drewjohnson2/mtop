#include <arena.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/monitor.h"
#include "../../include/sorting.h"

#define MAX_PROC_REGIONS_ALLOCD 3


static int _copy_if_tracked_stat(char *buf);

static void _fetch_proc_pid_stat(
    Arena *prcArena,
    ProcessList **item,
    char *statPath,
    char *statusPath
)
{
    char statBuffer[1024];
    char statusBuffer[1024];
    uid_t uid = getuid();
    u64 vmRss = 0;
    
    FILE *statusFile = fopen(statusPath, "r");
    
    if (!statusFile) return;
    
    while (fgets(statusBuffer, sizeof(statusBuffer), statusFile))
    {
	u32 fuid = 0;
    
    	if (sscanf(statusBuffer, "Uid:\t%d", &fuid) <= 0) continue;
    	else if (fuid != (u32)uid)
    	{
	    fclose(statusFile);
    
	    return;
    	}
    	else break;
    }
    
    // add something else in here that checks for a system processs
    
    // I need this for calculating the process memory percentage
    // possibly move fclose() out of the loop
    while (fgets(statusBuffer, sizeof(statusBuffer), statusFile))
    {
    	if (sscanf(statusBuffer, "VmRSS:\t%lu kB\n", &vmRss) <= 0) continue;
    	else 
    	{
	    fclose(statusFile);
    
	    break;
    	}
    }
    
    FILE *statFile = fopen(statPath, "r");	
    
    if (!statFile) return;
    
    *item = a_alloc(
	prcArena,
    	sizeof(ProcessList),
    	__alignof(ProcessList)
    );
    
    fgets(statBuffer, sizeof(statBuffer), statFile);
    
    char name[99];
    
    sscanf(statBuffer,
	"%hu %98s %*c %*d %*d "
    	"%*d %*d %*d %*u %*lu "
    	"%*lu %*lu %*lu %lu %lu ",
    	&(*item)->pid, name,
    	&(*item)->utime, &(*item)->stime
    );
    
    size_t len = strlen(name) - 2;
    
    strncpy((*item)->procName, name + 1, len);
    
    (*item)->vmRss = vmRss;
    
    fclose(statFile);
}

ProcessStats * get_processes(
    Arena *procArena,
    int (*sortFunc)(const void *, const void *)
) 
{
    DIR *directory;
    struct dirent *dp;
    
    if ((directory = opendir("/proc")) == NULL) exit(1);
    
    if (procArena->regionsAllocated > MAX_PROC_REGIONS_ALLOCD)
    {
	r_free_head(procArena);
    }
    
    ProcessStats *procStats = a_alloc(
	procArena,
    	sizeof(ProcessStats),
    	__alignof(ProcessStats)
    );
    procStats->processes = a_alloc(
	procArena,
	sizeof(ProcessList *) * MAX_PROCS,
	__alignof(ProcessList *)
    );
    
    for (procStats->count = 0; (dp = readdir(directory)) != NULL;)
    {
	if (procStats->count > MAX_PROCS - 1) break;
    
    	char statPath[32];
    	char statusPath[32];
    	u8 skip = atoi(dp->d_name) == 0;
    
    	if (skip || dp->d_type != DT_DIR) continue;
    
    	snprintf(statPath, sizeof(statPath), "/proc/%s/stat", dp->d_name);
    	snprintf(statusPath, sizeof(statusPath), "/proc/%s/status", dp->d_name);
    
    	_fetch_proc_pid_stat(
	   procArena,
    	   &procStats->processes[procStats->count],
    	   statPath,
    	   statusPath
    	);
    
    	if (procStats->processes[procStats->count] != NULL) procStats->count++;
    }
    
    procStats->cpuTimeAtSample = cpu_time_now();
    
    qsort(procStats->processes, procStats->count, sizeof(ProcessList *), sortFunc);
    
    closedir(directory);
    
    return procStats;
}


void populate_SD_by_pid(volatile ProcessInfoSharedData *prcInfoSd)
{
    char statusPath[32];
    char statusBuffer[1024];

    snprintf(statusPath, sizeof(statusPath), "/proc/%d/status", prcInfoSd->pidToFetch);

    FILE *statusFile = fopen(statusPath, "r");
    
    if (!statusFile) return;
    
    size_t i = 0;

    while (fgets(statusBuffer, sizeof(statusBuffer), statusFile))
    {
	int isTracked = _copy_if_tracked_stat(statusBuffer);

	if (isTracked) 
	{
	    strcpy(prcInfoSd->info->stats[i++], statusBuffer);
	}
    }

    fclose(statusFile);
}

static int _copy_if_tracked_stat(char *buf)
{
    char **res = bsearch(buf, trackedStats, 19, sizeof(char *), prc_tracked_stat_cmp);

    return res ? 1 : 0;
}
