#ifndef MONITOR_H
#define MONITOR_H

#include <arena.h>
#include <stdint.h>
#include <stdio.h>

#include "mt_type_defs.h"

#define MAX_PROCS 50 

typedef struct _proc_list
{
	u16 pid;
	char procName[99];
	u64 utime;
	u64 stime;
	u64 vmRss;

} ProcessList;

typedef struct _proc_stats
{
	size_t count;
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
	u8 cpuNumber;
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

#define CALC_PRC_CPU_USAGE_PCT(prev, cur, pct, prevCpuTime, curCpuTime) \
	do { \
		u8 cpuCount = sysconf(_SC_NPROCESSORS_ONLN); \
		float elapsedCpuTime = curCpuTime - prevCpuTime; \
		float procCpuTime = (cur->stime + cur->utime) - (prev->stime + prev->utime); \
		\
		pct = elapsedCpuTime > 0 ? \
			(procCpuTime / elapsedCpuTime) * 100 * cpuCount \
			: 0; \
	} while(0)\

static inline u64 cpu_time_now()
{
	FILE *f = fopen("/proc/stat", "r");
	char buffer[512];

	if (!f) return 0;

	u64 user, nice, system, idle, ioWait, irq, softIrq, steal;

	fgets(buffer, sizeof(buffer), f);

	sscanf(buffer, 
		"cpu  %lu %lu %lu %lu %lu %lu %lu %lu\n", 
		&user, &nice, &system, &idle, &ioWait,
		&irq, &softIrq, &steal
	);

	fclose(f);

	return user + nice + system + idle + ioWait + irq + softIrq + steal;
}

CpuStats * fetch_cpu_stats(Arena *arena);
MemoryStats * fetch_memory_stats(Arena *arena);
ProcessStats * get_processes(
	Arena *procArena,
	int (*sortFunc)(const void *, const void *)
);

#endif

