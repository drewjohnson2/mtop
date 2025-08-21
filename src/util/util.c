#include "../../include/util.h"

u8 win_compare_above(WindowData *cmp, WindowData *cur)
{
    return cmp->windowY < cur->windowY;
}

u8 win_compare_below(WindowData *cmp, WindowData *cur)
{
    return cmp->windowY > cur->windowY;
}

u8 win_compare_left(WindowData *cmp, WindowData *cur)
{
    return cmp->windowX < cur->windowX;
}

u8 win_compare_right(WindowData *cmp, WindowData *cur)
{
    return cmp->windowX > cur->windowX;
}
