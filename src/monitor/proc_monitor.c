#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "../include/monitor/proc_monitor.h"

static void _fetch_proc_pid_stat(char *dirName, PROC_STATS *stats)
{
	char buffer[1024];

	FILE *f = fopen(dirName, "r");	

	if (!f) return;

	fgets(buffer, sizeof(buffer), f);

	sscanf(buffer,
		"%d %s %*c %*d %*d "
		"%*d %*d %*d %*u %*lu "
		"%*lu %*lu %*lu %lu %lu ",
		&stats->pid, stats->procName,
		&stats->utime, &stats->stime
		);

	char *start = strchr(stats->procName, '(') + 1;
	char *end = strrchr(stats->procName, ')');

	memcpy(stats->procName, start, end - start);

	stats->procName[end - start] = '\0';

	fclose(f);
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
	PROC_STATS **procs = a_alloc(procArena, sizeof(PROC_STATS *) * 50, __alignof(PROC_STATS));

	while ((dp = readdir(directory)) != NULL)
	{
		char path[32];

		int skip = (strcmp(dp->d_name, ".") == 0) ||
			(strcmp(dp->d_name, "..") == 0) ||
			(strcmp(dp->d_name, "1") == 0) ||
			(strcmp(dp->d_name, "bus") == 0) ||
			(strcmp(dp->d_name, "sys") == 0) ||
			(strcmp(dp->d_name, "tty") == 0);

		if (skip || dp->d_type != DT_DIR) continue;

		snprintf(path, sizeof(path), "/proc/%s/stat", dp->d_name);

		// open directory
		procs[i] = a_alloc(procArena, sizeof(PROC_STATS), __alignof(PROC_STATS));

		_fetch_proc_pid_stat(path, procs[i]);

		i++;
	}

	for (int y = 0; y < i; y++)
	{
		if (procs[y] == NULL) break;
		printf("PID: %d\nProcess Name: %s\n", procs[y]->pid, procs[y]->procName);
		printf("utime: %lu\nstime: %lu\n\n", procs[y]->utime, procs[y]->stime);
	}

	return procs;
}
