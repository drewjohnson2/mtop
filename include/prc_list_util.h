#include <ncurses.h>

#include "../include/window.h"

void set_start_end_idx(ProcessListState *state); 
void adjust_state(ProcessListState *state, ProcessStats *stats);
void read_input(
    WINDOW *win,
    ProcessListState *state,
    DisplayItems *di,
    ProcessStatsViewData **vd
);
