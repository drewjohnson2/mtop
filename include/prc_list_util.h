#include <ncurses.h>

#include "../include/window.h"

void plu_setup_list_state(ProcessListState *listState, ProcessesSummary *curPrcs, const WindowData *prcWin);
void plu_set_start_end_idx(ProcessListState *state); 
void plu_adjust_state(ProcessListState *state, ProcessesSummary *stats);
