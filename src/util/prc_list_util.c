#include <ncurses.h>

#include "../../include/prc_list_util.h"
#include "../../include/monitor.h"

void plu_setup_list_state(ProcessListState *listState, ProcessesSummary *curPrcs, const WindowData *prcWin)
{
    listState->cmdBuffer = '\0';
    listState->timeoutActive = false;
    listState->pageStartIdx = 0;
    listState->count = curPrcs->count;
    listState->pageSize = prcWin->wHeight - 5;
    listState->selectedIndex = listState->selectedIndex > listState->pageSize - 1 ?
	listState->pageSize - 1 :
	listState->selectedIndex;
    listState->totalPages = listState->count / listState->pageSize;
    listState->selectedPid = 0;
    listState->activePage = 0;

    if (listState->count % listState->pageSize > 0) listState->totalPages++;

    listState->pageEndIdx = listState->pageSize - 1;

    if (listState->pageEndIdx >= listState->count)
		listState->pageEndIdx = listState->count - 1;

    listState->sortFn = vd_name_compare_fn;
    listState->sortOrder = PRC_NAME;
    listState->infoVisible = false;
}

void plu_set_start_end_idx(ProcessListState *state) 
{
    s8 isLastPage = state->activePage == state->totalPages - 1;
    size_t lastPageEnd = state->count - 1;
    size_t pageEndIdx = (state->activePage * state->pageSize - 1) + state->pageSize;

    state->pageStartIdx = state->pageSize * state->activePage;
    state->pageEndIdx = isLastPage ?
		lastPageEnd : 
		pageEndIdx;

    if (state->pageEndIdx < state->selectedIndex)
    {
		state->selectedIndex = state->pageEndIdx;
    }
}

void plu_adjust_state(ProcessListState *state, ProcessesSummary *stats)
{
    if (state->count == (s8)stats->count) return;
    
    state->count = stats->count;

    u8 updatedPageCount = state->count / state->pageSize;

    if (state->count % state->pageSize > 0) updatedPageCount++;

    state->totalPages = updatedPageCount; 

    if (state->activePage > state->totalPages - 1) state->activePage = state->totalPages - 1;

    plu_set_start_end_idx(state);

    state->selectedIndex = state->selectedIndex > state->count - 1 ?
	state->count - 1 :
	state->selectedIndex;
}
