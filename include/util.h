#ifndef UTIL_H
#define UTIL_H

#include "window.h"
#include "mt_type_defs.h"

u8 win_compare_above(WindowData *cmp, WindowData *cur);
u8 win_compare_below(WindowData *cmp, WindowData *cur);
u8 win_compare_left(WindowData *cmp, WindowData *cur);
u8 win_compare_right(WindowData *cmp, WindowData *cur);
LayoutOrientation get_orientation_for_layout(Layout layout);

#endif
