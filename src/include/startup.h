#ifndef STARTUP_H
#define STARTUP_H

#include <pthread.h>

#include "monitor.h"

extern volatile PROC_STATS **procStats;

void run();

#endif
