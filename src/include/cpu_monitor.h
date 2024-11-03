#ifndef CPU_MONITOR_H
#define CPU_MONITOR_H

#include <stdint.h>
#include <arena.h>

typedef struct _cpu_stats 
{
	int cpuNumber;
	unsigned long long int user;
	unsigned long long int nice;
	unsigned long long int system;
	unsigned long long int idle;
	unsigned long long int ioWait;
	unsigned long long int irq;
	unsigned long long int softIrq;
	unsigned long long int steal;
	unsigned long long int guest;
	unsigned long long int guestNice;
} CPU_STATS;

CPU_STATS * fetch_cpu_stats(Arena *arena);
float calculate_cpu_usage(CPU_STATS *prev, CPU_STATS *cur);
#endif
