#include <arena.h>
#include <stdio.h>

#include "../../include/monitor.h"

#define MAX_MEM_REGIONS_ALLOCD 9

static void _parse_stat(volatile MemoryStats *stat, char *buffer);

void fetch_memory_stats(volatile MemoryStats *memStats)
{
    FILE *f = fopen("/proc/meminfo", "r");
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), f))
    {
	_parse_stat(memStats, buffer);
    }
    
    fclose(f);
}

static void _parse_stat(volatile MemoryStats *stat, char *buffer)
{
    if (sscanf(buffer, "MemTotal: %lu kB\n", &stat->memTotal) == 1) return;
    else if (sscanf(buffer, "MemFree: %lu kB\n", &stat->memFree) == 1) return;
    else if (sscanf(buffer, "Buffers: %lu kB\n", &stat->buffers) == 1) return;
    else if (sscanf(buffer, "Cached: %lu kB\n", &stat->cachedMem) == 1) return;
    else if (sscanf(buffer, "Shmem: %lu kB\n", &stat->shared) == 1) return;
    else if (sscanf(buffer, "SReclaimable: %lu kB\n", &stat->sReclaimable) == 1) return;
}
