#ifndef STARTUP_H
#define STARTUP_H

#include "../cpu_monitor.h"
#include "../ram_monitor.h"

extern CPU_STATS *prevStats;
extern CPU_STATS *curStats;
extern RAM_STATS *memStats;

void run();

#endif
