#include <ncurses.h>

#include "../include/window.h"

void setup_list_state(ProcessListState *listState, ProcessesSummary *curPrcs, const WindowData *prcWin);
void set_start_end_idx(ProcessListState *state); 
void adjust_state(ProcessListState *state, ProcessesSummary *stats);
