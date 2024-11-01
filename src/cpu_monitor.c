#include <arena.h>
#include <stdio.h>
#include <unistd.h>

#include "include/cpu_monitor.h"

static void _parse_stat(CPU_STATS *stat, char *buffer, size_t lineNumber);
static inline int _str_starts_with(char *str1, char *str2, size_t len);

CPU_STATS ** fetch_cpu_stats(Arena *arena) 
{
	FILE *f = fopen("/proc/stat", "r");
	char buffer[256];
	size_t lineNumber = 0;
	long nCores = sysconf(_SC_NPROCESSORS_ONLN);

	CPU_STATS **stats = a_alloc(
		arena,
		sizeof(CPU_STATS *) * nCores,
		_Alignof(CPU_STATS *)
	);

	while (fgets(buffer, sizeof(buffer), f))
	{
		if (_str_starts_with(buffer, "cpu", 3) != 0) break;

		stats[lineNumber] = a_alloc(arena, sizeof(CPU_STATS), _Alignof(CPU_STATS));

		_parse_stat(stats[lineNumber], buffer, lineNumber);

		lineNumber++;
	}

	return stats;
}

static void _parse_stat(CPU_STATS *stat, char *buffer, size_t lineNumber)
{
	if (lineNumber == 0)
	{
		sscanf(buffer, 
			"cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n", 
			&stat->user, &stat->nice, &stat->system, &stat->idle, &stat->ioWait,
			&stat->irq, &stat->softIrq, &stat->steal, &stat->guest, &stat->guestNice
		);
	}
	else 
	{
		sscanf(buffer, 
			"cpu%4u %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n", 
			&stat->cpuNumber, &stat->user, &stat->nice, &stat->system, &stat->idle, &stat->ioWait,
			&stat->irq, &stat->softIrq, &stat->steal, &stat->guest, &stat->guestNice
		); 
	}
}

static inline int _str_starts_with(char *str1, char *str2, size_t len)
{
	char hold[len + 1];

	for (size_t i = 0; i < len; i++) hold[i] = str1[i];

	hold[len] = '\0';

	return strcmp(hold, str2);
}
