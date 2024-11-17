#ifndef STARTUP_H
#define STARTUP_H

#include <pthread.h>

#include "../monitor/proc_monitor.h"

extern PROC_STATS **procStats;

void run();

#endif
