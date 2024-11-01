#include <arena.h>
#include <stdio.h>

#include "include/cpu_monitor.h"

static int _parse_stat(CPU_STATS *stat, char *buffer, size_t lineNumber);

int main() 
{
	Arena arena = a_new(2048);

	CPU_STATS **stats = a_alloc(
		&arena,
		sizeof(CPU_STATS *) * 9,
		_Alignof(CPU_STATS *)
	);

	FILE *f = fopen("/proc/stat", "r");
	char buffer[256];
	size_t lineNumber = 0;

	while (fgets(buffer, sizeof(buffer), f))
	{
		// don't do this. Just check if start of the string is cpu
		if (lineNumber > 8) break;

		stats[lineNumber] = a_alloc(&arena, sizeof(CPU_STATS), _Alignof(CPU_STATS));

		int hi = _parse_stat(stats[lineNumber], buffer, lineNumber);

		printf("%d\n", hi);

		lineNumber++;
	}

	printf("%u %u %u %u %u %u %u %u %u %u\n", 
		stats[0]->user, stats[0]->nice, stats[0]->system, stats[0]->idle, stats[0]->ioWait,
		stats[0]->irq, stats[0]->softIrq, stats[0]->steal, stats[0]->guest, stats[0]->guestNice);

	for (int i = 1; i < 9; i++)
	{
		printf("%u %u %u %u %u %u %u %u %u %u\n",
			stats[i]->user, stats[i]->nice, stats[i]->system, stats[i]->idle, stats[i]->ioWait,
			stats[i]->irq, stats[i]->softIrq, stats[i]->steal, stats[i]->guest, stats[i]->guestNice);	
	}

	a_free(&arena);
}

static int _parse_stat(CPU_STATS *stat, char *buffer, size_t lineNumber)
{
	// make this... more gooder
	if (lineNumber == 0)
	{
		int why = sscanf(buffer, 
			"cpu  %16u %16u %16u %16u %16u %16u %16u %16u %16u %16u\n", 
			&stat->user, &stat->nice, &stat->system, &stat->idle, &stat->ioWait,
			&stat->irq, &stat->softIrq, &stat->steal, &stat->guest, &stat->guestNice
		);

		return why;
	}
	else 
	{
		int what = sscanf(buffer, 
			"cpu%4u %16u %16u %16u %16u %16u %16u %16u %16u %16u %16u\n", 
			&stat->cpuNumber, &stat->user, &stat->nice, &stat->system, &stat->idle, &stat->ioWait,
			&stat->irq, &stat->softIrq, &stat->steal, &stat->guest, &stat->guestNice
		); 

		return what;
	}

	return -1;
}
