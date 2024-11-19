#ifndef MONITOR_H
#define MONITOR_H

#include <arena.h>

#define MAX_PROCS 50

typedef struct _proc_stats
{
	int pid;
	char procName[99];
	unsigned long utime;
	unsigned long stime;

} ProcessStats;

typedef struct _mem_stats
{
	unsigned long long int memTotal;
	unsigned long long int memFree;
	unsigned long long int cachedMem;
	unsigned long long int sReclaimable;
	unsigned long long int shared;
	unsigned long long int buffers;
} MemoryStats;

typedef struct _cpu_stats 
{
	int cpuNumber;
	unsigned long long int user;
	unsigned long long int nice;
	unsigned long long int system;
	unsigned long long int idle;
	unsigned long long int ioWait;
	unsigned long long int irq;
	unsigned long long int softIrq;
	unsigned long long int steal;
	unsigned long long int guest;
	unsigned long long int guestNice;
} CpuStats;

#define CALCULATE_MEMORY_USAGE(stats, percentage) \
	do { \
		unsigned long long usedDiff = stats->memFree + stats->cachedMem \
			+ stats->sReclaimable + stats->buffers; \
		\
		percentage = (stats->memTotal - usedDiff) / (float)stats->memTotal; \
	} while(0) \

#define CALCULATE_CPU_PERCENTAGE(prev, cur, percentage) \
	do { \
		unsigned long long int prevIdle, idle, prevActive, active; \
		unsigned long long int prevTotal, total; \
		unsigned long long int totalDiff, idleDiff; \
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


MemoryStats * fetch_memory_stats(Arena *arena);
void get_processes(Arena *procArena,int (*sortFunc)(const void *, const void *));
CpuStats * fetch_cpu_stats(Arena *arena);

#endif

