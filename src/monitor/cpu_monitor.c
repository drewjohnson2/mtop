#include <arena.h>
#include <stdio.h>

#include "../include/monitor.h"

#define MAX_CPU_REGIONS_ALLOCD 5

CpuStats * fetch_cpu_stats(Arena *arena) 
{
    FILE *f = fopen("/proc/stat", "r");
    char buffer[512];

    if (arena->regionsAllocated > MAX_CPU_REGIONS_ALLOCD) r_free_head(arena);
    
    CpuStats *stat = a_alloc(
	arena,
	sizeof(CpuStats),
	__alignof(CpuStats) 
    );
    
    fgets(buffer, sizeof(buffer), f);
    
    sscanf(buffer, 
	"cpu  %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n", 
	&stat->user, &stat->nice, &stat->system, &stat->idle, &stat->ioWait,
	&stat->irq, &stat->softIrq, &stat->steal, &stat->guest, &stat->guestNice
    );
    
    fclose(f);
    
    return stat;
}
