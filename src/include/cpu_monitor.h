#ifndef CPU_MONITOR_H
#define CPU_MONITOR_H

#include <stdint.h>
#include <arena.h>

typedef struct _cpu_stats 
{
	int cpuNumber;
	long long int user;
	long long int nice;
	long long int system;
	long long int idle;
	long long int ioWait;
	long long int irq;
	long long int softIrq;
	long long int steal;
	long long int guest;
	long long int guestNice;
} CPU_STATS;

CPU_STATS ** fetch_cpu_stats(Arena *arena);

#endif
