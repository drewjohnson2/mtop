#ifndef HELPERS_H
#define HELPERS_H

#include "window.h"

void platform_print_fields(
    ProcessStatsViewData *vd,
    const WindowData *wd,
    const MT_Color_Pairs boxPair,
    u8 posX,
    u8 posY
);

void platform_set_vd(ProcessStatsViewData *vd, Process *cur);

#endif
