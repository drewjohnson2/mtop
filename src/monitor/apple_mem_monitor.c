#include <mach/mach.h>
#include <mach/mach_host.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdint.h>

#include "../../include/monitor.h"

void mm_fetch_memory_stats(volatile MemoryStats *memStats)
{
    vm_statistics64_data_t vmStat;
    vm_size_t pageSize;
    uint64_t memTotal = 0;
    size_t size = sizeof(memTotal);
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

    memStats->memTotal = 0;
    memStats->memFree = 0;
    memStats->cachedMem = 0;
    memStats->buffers = 0;
    memStats->shared = 0;
    memStats->sReclaimable = 0;

    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vmStat, &count) == KERN_SUCCESS)
    {
        host_page_size(mach_host_self(), &pageSize);
        sysctlbyname("hw.memsize", &memTotal, &size, NULL, 0);

        memStats->memTotal = memTotal;
        memStats->memFree = vmStat.free_count;
        memStats->cachedMem = vmStat.inactive_count * pageSize;
    }
}
