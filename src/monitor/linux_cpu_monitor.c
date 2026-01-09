#include <arena.h>
#include <stdio.h>
#include <proc/readproc.h>

#include "../../include/monitor.h"

#define MAX_CPU_REGIONS_ALLOCD 5

void cm_fetch_cpu_stats(CpuStats *stat) 
{
    FILE *f = fopen("/proc/stat", "r");
    char buffer[512];

    fgets(buffer, sizeof(buffer), f);
    
    sscanf(buffer, 
		"cpu  %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n", 
		&stat->user, &stat->nice, &stat->system, &stat->idle, &stat->ioWait,
		&stat->irq, &stat->softIrq, &stat->steal, &stat->guest, &stat->guestNice
    );
    
    fclose(f);
}
