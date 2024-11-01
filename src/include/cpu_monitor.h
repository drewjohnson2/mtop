#ifndef CPU_MONITOR_H
#define CPU_MONITOR_H

#include <stdint.h>

typedef struct _cpu_stats 
{
	int cpuNumber;
	uint32_t user;
	uint32_t nice;
	uint32_t system;
	uint32_t idle;
	uint32_t ioWait;
	uint32_t irq;
	uint32_t softIrq;
	uint32_t steal;
	uint32_t guest;
	uint32_t guestNice;
} CPU_STATS;

#endif
