#ifndef PROC_MONITOR_H
#define PROC_MONITOR_H

#include <arena.h>

typedef struct _proc_stats
{
	int pid;
	char procName[200];
	unsigned long utime;
	unsigned long stime;

} PROC_STATS;

PROC_STATS ** get_processes(
	Arena *procArena,
	int (*f)(const void *, const void *)
);

#endif

