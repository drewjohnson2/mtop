#ifndef CPU_MONITOR_H
#define CPU_MONITOR_H

#include <stdint.h>
#include <arena.h>

#define CALCULATE_CPU_PERCENTAGE(prev, cur, percentage) \
	do { \
		unsigned long long int prevIdle, idle, prevActive, active; \
		unsigned long long int prevTotal, total; \
		unsigned long long int totalDiff, idleDiff; \
		\
		prevIdle = prev->idle + prev->ioWait; \
		idle = cur->idle + cur->ioWait; \
		\
		prevActive = prev->user + prev->nice + prev->system + prev->irq \
			+ prev->softIrq + prev->steal; \
		\
		active = cur ->user + cur->nice + cur->system + cur->irq \
			+ cur->softIrq + cur->steal; \
		\
		prevTotal = prevIdle + prevActive; \
		total = idle + active; \
		\
		totalDiff = total - prevTotal; \
		idleDiff = idle - prevIdle; \
		\
		percentage = totalDiff != 0 ? \
			(totalDiff - idleDiff) / (float)totalDiff : \
			0; \
		\
	} while(0)\

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

#endif
