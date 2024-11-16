#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/monitor/proc_monitor.h"

static void _fetch_proc_pid_stat(
	Arena *procArena,
	char *statPath,
	char *statusPath,
	PROC_STATS **stats,
	uid_t uid
)
{
	char statBuffer[1024];
	char statusBuffer[1024];

	FILE *statusFile = fopen(statusPath, "r");

	if (!statusFile) return;

	while(fgets(statusBuffer, sizeof(statusBuffer), statusFile))
	{
	  	int fuid = 0;

		if (sscanf(statusBuffer, "Uid:\t%d", &fuid) <= 0) continue;
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

	(*stats) = a_alloc(procArena, sizeof(PROC_STATS), __alignof(PROC_STATS));

	fgets(statBuffer, sizeof(statBuffer), statFile);

	sscanf(statBuffer,
		"%d %s %*c %*d %*d "
		"%*d %*d %*d %*u %*lu "
		"%*lu %*lu %*lu %lu %lu ",
		&(*stats)->pid, (*stats)->procName,
		&(*stats)->utime, &(*stats)->stime
		);

	char *start = strchr((*stats)->procName, '(') + 1;
	char *end = strrchr((*stats)->procName, ')');

	memcpy((*stats)->procName, start, end - start);

	(*stats)->procName[end - start] = '\0';

	fclose(statFile);
}

PROC_STATS ** get_processes(Arena *procArena) 
{
	DIR *directory;
	struct dirent *dp;

	if ((directory = opendir("/proc")) == NULL)
	{
		exit(1);
	}

	int i = 0;
	uid_t uid = getuid();
	PROC_STATS **procs = a_alloc(procArena, sizeof(PROC_STATS *) * 50, __alignof(PROC_STATS));

	while ((dp = readdir(directory)) != NULL)
	{
		char statPath[32];
		char statusPath[32];

		int skip = atoi(dp->d_name) == 0;

		if (skip || dp->d_type != DT_DIR) continue;

		snprintf(statPath, sizeof(statPath), "/proc/%s/stat", dp->d_name);
		snprintf(statusPath, sizeof(statusPath), "/proc/%s/status", dp->d_name);

		_fetch_proc_pid_stat(procArena, statPath, statusPath, &procs[i], uid);

		if (procs[i] != NULL) i++;
	}

	// for (int y = 0; y < i; y++)
	// {
	// 	if (procs[y] == NULL) break;
	// 	printf("PID: %d\nProcess Name: %s\n", procs[y]->pid, procs[y]->procName);
	// 	printf("utime: %lu\nstime: %lu\n\n", procs[y]->utime, procs[y]->stime);
	// }

	return procs;
}
