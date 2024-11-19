#include <arena.h>
#include <stdio.h>

#include "../include/monitor.h"

static void _parse_stat(CPU_STATS *stat, char *buffer);

CPU_STATS * fetch_cpu_stats(Arena *arena) 
{
	FILE *f = fopen("/proc/stat", "r");
	char buffer[256];

	CPU_STATS *stat = a_alloc(
		arena,
		sizeof(CPU_STATS),
		__alignof(CPU_STATS) 
	);

	fgets(buffer, sizeof(buffer), f);

	_parse_stat(stat, buffer);

	fclose(f);

	return stat;
}

static void _parse_stat(CPU_STATS *stat, char *buffer)
{
	sscanf(buffer, 
		"cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n", 
		&stat->user, &stat->nice, &stat->system, &stat->idle, &stat->ioWait,
		&stat->irq, &stat->softIrq, &stat->steal, &stat->guest, &stat->guestNice
	);
}
