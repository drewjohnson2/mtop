#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>

#include "../../include/monitor.h"

void cm_fetch_cpu_stats(CpuStats *stat) 
{
    host_cpu_load_info_data_t cpuInfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

    stat->user = 0;
    stat->system = 0;
    stat->idle = 0;
    stat->nice = 0;
    stat->ioWait = 0;
    stat->irq = 0;
    stat->softIrq = 0;
    stat->steal = 0;
    stat->guest = 0;
    stat->guestNice = 0;

    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuInfo, &count) == KERN_SUCCESS)
    {
        stat->user = cpuInfo.cpu_ticks[CPU_STATE_USER];
        stat->system = cpuInfo.cpu_ticks[CPU_STATE_SYSTEM];
        stat->idle = cpuInfo.cpu_ticks[CPU_STATE_IDLE];
        stat->nice = cpuInfo.cpu_ticks[CPU_STATE_NICE];
    }
}
