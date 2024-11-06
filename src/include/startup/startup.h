#ifndef STARTUP_H
#define STARTUP_H

#include "../cpu_monitor.h"
#include "../ram_monitor.h"

typedef struct _thread_shared_data 
{
	CPU_STATS *prev;
	CPU_STATS *cur;
	RAM_STATS *memStats;
	short readingFile;
} shared_data;

void run();

#endif
