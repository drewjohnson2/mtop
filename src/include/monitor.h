#ifndef MONITOR_H
#define MONITOR_H

#include <arena.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_PROCS 50

typedef unsigned long long u64;
typedef unsigned long u32;

typedef struct _proc_list
{
	int pid;
	char procName[99];
	u32 utime;
	u32 stime;

} ProcessList;

typedef struct _proc_stats
{
	int count;
	u64 cpuTimeAtSample;
	ProcessList **processes;
} ProcessStats;

typedef struct _mem_stats
{
	u64 memTotal;
	u64 memFree;
	u64 cachedMem;
	u64 sReclaimable;
	u64 shared;
	u64 buffers;
} MemoryStats;

typedef struct _cpu_stats 
{
	int cpuNumber;
	u64 user;
	u64 nice;
	u64 system;
	u64 idle;
	u64 ioWait;
	u64 irq;
	u64 softIrq;
	u64 steal;
	u64 guest;
	u64 guestNice;
} CpuStats;

#define CALCULATE_MEMORY_USAGE(stats, percentage) \
	do { \
		u64 usedDiff = stats->memFree + stats->cachedMem \
			+ stats->sReclaimable + stats->buffers; \
		\
		percentage = (stats->memTotal - usedDiff) / (float)stats->memTotal; \
	} while(0) \

// found this calculation at https://stackoverflow.com/a/23376195
#define CALCULATE_CPU_PERCENTAGE(prev, cur, percentage) \
	do { \
		u64 prevIdle, idle, prevActive, active; \
		u64 prevTotal, total; \
		u64 totalDiff, idleDiff; \
		\
		prevIdle = prev->idle + prev->ioWait; \
		idle = cur->idle + cur->ioWait; \
		\
		prevActive = prev->user + prev->nice + prev->system + prev->irq \
			+ prev->softIrq + prev->steal; \
		\
		active = cur ->user + cur->nice + cur->system + cur->irq \
			+ cur->softIrq + cur->steal; \
		\
		prevTotal = prevIdle + prevActive; \
		total = idle + active; \
		\
		totalDiff = total - prevTotal; \
		idleDiff = idle - prevIdle; \
		\
		percentage = totalDiff != 0 ? \
			(totalDiff - idleDiff) / (float)totalDiff : \
			0; \
		\
	} while(0)\

static inline u64 cpu_time_now()
{
	FILE *f = fopen("/proc/stat", "r");
	char buffer[512];

	u64 user, nice, system, idle, ioWait, irq, softIrq, steal;

	fgets(buffer, sizeof(buffer), f);

	sscanf(buffer, 
		"cpu  %llu %llu %llu %llu %llu %llu %llu %llu\n", 
		&user, &nice, &system, &idle, &ioWait,
		&irq, &softIrq, &steal
	);

	return user + nice + system + idle + ioWait + irq + softIrq + steal;
}

CpuStats * fetch_cpu_stats(Arena *arena);
MemoryStats * fetch_memory_stats(Arena *arena);
void get_processes(Arena *procArena,int (*sortFunc)(const void *, const void *));

#endif

