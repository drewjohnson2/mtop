#include <arena.h>
#include <stdio.h>
//#include <unistd.h>

#include "../include/cpu_monitor.h"

static void _parse_stat(CPU_STATS *stat, char *buffer);

CPU_STATS * fetch_cpu_stats(Arena *arena) 
{
	FILE *f = fopen("/proc/stat", "r");
	char buffer[256];

	CPU_STATS *stat = a_alloc(
		arena,
		sizeof(CPU_STATS),
		_Alignof(CPU_STATS) 
	);

	fgets(buffer, sizeof(buffer), f);

	_parse_stat(stat, buffer);

	fclose(f);

	return stat;
}

float calculate_cpu_usage(CPU_STATS *prev, CPU_STATS *cur)
{
	unsigned long long int prevIdle, idle, prevActive, active;
	unsigned long long int prevTotal, total;
	unsigned long long int totalDiff, idleDiff;

	prevIdle = prev->idle + prev->ioWait;
	idle = cur->idle + cur->ioWait;

	prevActive = prev->user + prev->nice + prev->system + prev->irq + prev->softIrq + prev->steal;
	active = cur ->user + cur->nice + cur->system + cur->irq + cur->softIrq + cur->steal;

	prevTotal = prevIdle + prevActive;
	total = idle + active;

	totalDiff = total - prevTotal;
	idleDiff = idle - prevIdle;

	return (totalDiff - idleDiff) / (float)totalDiff;
}

static void _parse_stat(CPU_STATS *stat, char *buffer)
{
	sscanf(buffer, 
		"cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n", 
		&stat->user, &stat->nice, &stat->system, &stat->idle, &stat->ioWait,
		&stat->irq, &stat->softIrq, &stat->steal, &stat->guest, &stat->guestNice
	);
}
