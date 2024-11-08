#ifndef RAM_MONITOR_H
#define RAM_MONITOR_H

#include <arena.h>

#define CALCULATE_MEMORY_USAGE(stats, percentage) \
	do { \
		unsigned long long usedDiff = stats->memFree + stats->cachedMem \
			+ stats->sReclaimable + stats->buffers; \
		\
		percentage = (stats->memTotal - usedDiff) / (float)stats->memTotal; \
	} while(0) \

typedef struct _mem_stats
{
	unsigned long long int memTotal;
	unsigned long long int memFree;
	unsigned long long int cachedMem;
	unsigned long long int sReclaimable;
	unsigned long long int shared;
	unsigned long long int buffers;
} MEMORY_STATS;

MEMORY_STATS * fetch_memory_stats(Arena *arena);

#endif

