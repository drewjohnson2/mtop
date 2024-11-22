#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/monitor.h"
#include "../include/startup.h"

static void _fetch_proc_pid_stat(
	Arena *procArena,
	char *statPath,
	char *statusPath
)
{
	char statBuffer[1024];
	char statusBuffer[1024];
	int index = procStats->count;
	uid_t uid = getuid();

	FILE *statusFile = fopen(statusPath, "r");

	if (!statusFile) return;

	while(fgets(statusBuffer, sizeof(statusBuffer), statusFile))
	{
	  	int fuid = 0;

		if (sscanf(statusBuffer, "Uid:\t%d", &fuid) <= 0) 
		{
			continue;
		}
		else if (fuid != (int)uid)
		{
			fclose(statusFile);

			return;
		}
		else 
		{
			fclose(statusFile);
		
			break;
		}
	}

	FILE *statFile = fopen(statPath, "r");	

	if (!statFile) return;

	procStats->processes[index] = a_alloc(
		procArena,
		sizeof(ProcessList),
		__alignof(ProcessList)
	);

	fgets(statBuffer, sizeof(statBuffer), statFile);

	sscanf(statBuffer,
		"%d %*[(]%99[^)'] %*c %*d %*d "
		"%*d %*d %*d %*u %*lu "
		"%*lu %*lu %*lu %lu %lu ",
		&procStats->processes[index]->pid, procStats->processes[index]->procName,
		&procStats->processes[index]->utime, &procStats->processes[index]->stime
		);

	fclose(statFile);
}

void get_processes(
	Arena *procArena,
	int (*sortFunc)(const void *, const void *)
) 
{
	DIR *directory;
	struct dirent *dp;

	if ((directory = opendir("/proc")) == NULL) exit(1);

	a_free(procArena);

	*procArena = a_new(512);

	procStats = a_alloc(
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

		int skip = atoi(dp->d_name) == 0;

		if (skip || dp->d_type != DT_DIR) continue;

		snprintf(statPath, sizeof(statPath), "/proc/%s/stat", dp->d_name);
		snprintf(statusPath, sizeof(statusPath), "/proc/%s/status", dp->d_name);

		_fetch_proc_pid_stat(procArena, statPath, statusPath);

		if (procStats->processes[procStats->count] != NULL) procStats->count++;
	}

	procStats->cpuTimeAtSample = cpu_time_now();

	qsort(procStats->processes, procStats->count, sizeof(ProcessList *), sortFunc);

	closedir(directory);
}
