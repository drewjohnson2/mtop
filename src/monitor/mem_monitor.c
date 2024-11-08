#include <stdio.h>
#include <string.h>

#include "../include/mem_monitor.h"

static inline int _str_starts_with(char *str1, char *str2, size_t len);
static void _parse_stat(MEMORY_STATS *stat, char *buffer);

MEMORY_STATS * fetch_memory_stats(Arena *arena)
{
	FILE *f = fopen("/proc/meminfo", "r");
	char buffer[256];

	MEMORY_STATS *stat = a_alloc(
		arena,
		sizeof(MEMORY_STATS),
		_Alignof(MEMORY_STATS)
	);

	while (fgets(buffer, sizeof(buffer), f))
	{
		if (_str_starts_with(buffer, "MemTotal:", 9) != 0 && 
	  		_str_starts_with(buffer, "MemFree:", 8) != 0)
			break;

		_parse_stat(stat, buffer);
	}

	fclose(f);

	return stat;
}

static void _parse_stat(MEMORY_STATS *stat, char *buffer)
{
	if (sscanf(buffer, "MemTotal: %llu kB\n", &stat->memTotal) == 1) return;
	else if (sscanf(buffer, "MemFree: %llu kB\n", &stat->memFree) == 1) return;
	else if (sscanf(buffer, "Buffers: %llu kB\n", &stat->buffers) == 1) return;
	else if (sscanf(buffer, "Cached: %llu kB\n", &stat->cachedMem) == 1) return;
	else if (sscanf(buffer, "Shmem: %llu kB\n", &stat->shared) == 1) return;
	else if (sscanf(buffer, "SReclaimable: %llu kB\n", &stat->sReclaimable) == 1) return;
}

static inline int _str_starts_with(char *str1, char *str2, size_t len)
{
	char hold[len + 1];

	for (size_t i = 0; i < len; i++) hold[i] = str1[i];

	hold[len] = '\0';

	return strcmp(hold, str2);
}
