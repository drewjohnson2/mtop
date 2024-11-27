#include <stdio.h>

#include "../include/monitor.h"

static void _parse_stat(MemoryStats *stat, char *buffer);

MemoryStats * fetch_memory_stats(Arena *arena)
{
	FILE *f = fopen("/proc/meminfo", "r");
	char buffer[256];

	MemoryStats *stat = a_alloc(
		arena,
		sizeof(MemoryStats),
		__alignof(MemoryStats)
	);

	while (fgets(buffer, sizeof(buffer), f))
	{
		_parse_stat(stat, buffer);
	}

	fclose(f);

	return stat;
}

static void _parse_stat(MemoryStats *stat, char *buffer)
{
	if (sscanf(buffer, "MemTotal: %lu kB\n", &stat->memTotal) == 1) return;
	else if (sscanf(buffer, "MemFree: %lu kB\n", &stat->memFree) == 1) return;
	else if (sscanf(buffer, "Buffers: %lu kB\n", &stat->buffers) == 1) return;
	else if (sscanf(buffer, "Cached: %lu kB\n", &stat->cachedMem) == 1) return;
	else if (sscanf(buffer, "Shmem: %lu kB\n", &stat->shared) == 1) return;
	else if (sscanf(buffer, "SReclaimable: %lu kB\n", &stat->sReclaimable) == 1) return;
}
