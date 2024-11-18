#ifndef PROC_MONITOR_H
#define PROC_MONITOR_H

#include <arena.h>

typedef struct _proc_stats
{
	int pid;
	char procName[99];
	unsigned long utime;
	unsigned long stime;

} PROC_STATS;

void get_processes(
	Arena *procArena,
	int (*sortFunc)(const void *, const void *)
);

#endif

