#ifndef RAM_MONITOR_H
#define RAM_MONITOR_H

#include <arena.h>

typedef struct _ram_stats
{
	unsigned long long int memTotal;
	unsigned long long int memFree;
	unsigned long long int cachedMem;
	unsigned long long int sReclaimable;
	unsigned long long int shared;
	unsigned long long int buffers;
} RAM_STATS;

RAM_STATS * fetch_ram_stats(Arena *arena);
float calculate_ram_usage(RAM_STATS *stats);

#endif

