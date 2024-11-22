#ifndef STARTUP_H
#define STARTUP_H

#include <pthread.h>

#include "monitor.h"

extern volatile ProcessStats *procStats;

void run();
void cleanup();

#endif
