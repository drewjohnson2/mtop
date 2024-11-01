#ifndef WINDOW_SETUP_H
#define WINDOW_SETUP_H

#include <ncurses.h>
#include <arena.h>

#include "window.h"

DISPLAY_ITEMS * init_display_items(Arena *arena);
void init_windows(DISPLAY_ITEMS *di);
void init_window_dimens(DISPLAY_ITEMS *di);
void init_ncurses(WINDOW_DATA *wd, SCREEN *screen);

#endif
