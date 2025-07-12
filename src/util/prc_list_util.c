
#include <ncurses.h>
#include <signal.h>

#include "../../include/prc_list_util.h"
#include "../../include/thread.h"
#include "../../include/monitor.h"
#include "../../include/startup.h"

typedef enum _nav_direction
{
    UP,
    DOWN,
    RIGHT,
    LEFT,
    JUMP_DOWN,
    JUMP_UP
} NavDirection;

static void _adjust_menu_index(NavDirection dir, ProcessListState *state);
static void _read_input(DisplayItems *di, s8 ch);

void setup_list_state(ProcessListState *listState, ProcessesSummary *curPrcs, const WindowData *prcWin)
{
    listState->cmdBuffer = '\0';
    listState->timeoutActive = 0;
    listState->selectedIndex = 0;
    listState->pageStartIdx = 0;
    listState->count = curPrcs->count;
    listState->pageSize = prcWin->wHeight - 5;
    listState->totalPages = listState->count / listState->pageSize;
    listState->selectedPid = 0;
    listState->activePage = 0;

    if (listState->count % listState->pageSize > 0) listState->totalPages++;

    listState->pageEndIdx = listState->pageSize - 1;

    if (listState->pageEndIdx > listState->count)
	listState->pageEndIdx = listState->count - 1;

    listState->sortFn = vd_name_compare_fn;
    listState->sortOrder = PRC_NAME;
    listState->infoVisible = 0;
}

void set_start_end_idx(ProcessListState *state) 
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

void adjust_state(ProcessListState *state, ProcessesSummary *stats)
{
    if (state->count == (s8)stats->count) return;
    
    state->count = stats->count;

    u8 updatedPageCount = state->count / state->pageSize;

    if (state->count % state->pageSize > 0) updatedPageCount++;

    state->totalPages = updatedPageCount; 

    if (state->activePage > state->totalPages - 1) state->activePage = state->totalPages - 1;

    set_start_end_idx(state);

    state->selectedIndex = state->selectedIndex > state->count - 1 ?
	state->count - 1 :
	state->selectedIndex;
}

// This function is kinda rough.
// I don't like it, but I am not going
// to rewrite it. I'm not even really sure
// how much I could do much to improve it. 
void read_input(
    WINDOW *win,
    ProcessListState *state,
    DisplayItems *di
)
{
    s8 ch = wgetch(win);

    if (!mtopSettings->activeWindows[PRC_WIN])
    {
	_read_input(di, ch);
	return;
    }

    if (state->infoVisible && (ch != 'b' && ch != 'q' && ch != 'o')) return;

    flushinp();

    u64 timeElapsedMs;
    struct timespec timeoutCurrent;
    
    if (state->timeoutActive)
    {
	clock_gettime(CLOCK_REALTIME, &timeoutCurrent);
    
    	timeElapsedMs = (timeoutCurrent.tv_sec - state->timeoutStart.tv_sec) * 1000
    	 + (timeoutCurrent.tv_nsec - state->timeoutStart.tv_nsec) 
    		/ 1000000;
    
    	if (timeElapsedMs > INPUT_TIMEOUT_MS)
    	{
	    state->cmdBuffer = '\0';
	    state->timeoutActive = 0;
    	}
    }
    
    if (ch == -1) return;
    
    switch (ch)
    {
	case 'j':
	    _adjust_menu_index(DOWN, state);

	    return;
    	case 'k':
	    _adjust_menu_index(UP, state);    

	    return;
	case 'l':
	    _adjust_menu_index(RIGHT, state);

	    return;
	case 'h':
	    _adjust_menu_index(LEFT, state);

	    return;
	case 4: // CTRL + D
	    _adjust_menu_index(JUMP_DOWN, state);
	
	    return;
	case 21: // CTRL + U
	    _adjust_menu_index(JUMP_UP, state);

	    return;
	case 'n':
	    if (state->sortOrder == PRC_NAME && sortDirection == ASC) sortDirection = DESC;
	    else sortDirection = ASC;

	    state->sortOrder = PRC_NAME;
	    state->sortFn = vd_name_compare_fn;

	    return;
	case 'p':
	    if (state->sortOrder == PID && sortDirection == ASC) sortDirection = DESC;
	    else sortDirection = ASC;

	    state->sortOrder = PID;
	    state->sortFn = vd_pid_compare_fn;

	    return;
	case 'c':
	    if (state->sortOrder == CPU && sortDirection == DESC) sortDirection = ASC;
	    else sortDirection = DESC;

	    state->sortOrder = CPU;
	    state->sortFn = vd_cpu_compare_fn;

	    return;
	case 'm':
	    if (state->sortOrder == MEM && sortDirection == DESC) sortDirection = ASC;
	    else sortDirection = DESC;

	    state->sortOrder = MEM;
	    state->sortFn = vd_mem_compare_fn;

	    return;
	case 'o':
	    di->optionsVisible = !di->optionsVisible;
	    
	    return;
	case 10:
	    state->infoVisible = 1;
	    
	    return;
	case 'b':
	    state->infoVisible = 0;

	    return;
    	case 'q':
	    SHUTDOWN_FLAG = 1;
	    return;
    	default:
	    break;
    }
    
    if (!state->cmdBuffer)
    {
	state->cmdBuffer = ch;
	state->timeoutActive = 1;
	clock_gettime(CLOCK_REALTIME, &state->timeoutStart);
	    
	return;
    }
    else if (ch != state->cmdBuffer) 
    {
	state->cmdBuffer = '\0';
	state->timeoutActive = 0;
    
	return;
    }
    else 
    {
	state->cmdBuffer = '\0';
	state->timeoutActive = 0;

	kill(state->selectedPid, SIGKILL);

	return;
    }
}

static void _adjust_menu_index(NavDirection dir, ProcessListState *state)
{
    s16 jumpValue;

    switch (dir) 
    {
	case UP:
    	    state->selectedIndex = state->selectedIndex > state->pageStartIdx ?
    	        state->selectedIndex - 1 :
    	        state->pageEndIdx;
	    break;
	case DOWN:
    	    state->selectedIndex = state->selectedIndex < state->pageEndIdx ?
    	        state->selectedIndex + 1 : 
    	        state->pageStartIdx;
	    break;
	case RIGHT:
    	    state->activePage = state->activePage < state->totalPages - 1 ?
    	        state->activePage + 1 :
    	        0;

    	    state->selectedIndex = state->activePage == 0 ?
    	        state->selectedIndex - state->pageStartIdx :
    	        state->selectedIndex + state->pageSize;

	    break;
	case LEFT:
    	    state->activePage = state->activePage > 0 ?
    	        state->activePage - 1 :
    	        state->totalPages - 1;

    	    state->selectedIndex = state->activePage == state->totalPages - 1 ?
    	        (state->pageSize * state->activePage) + state->selectedIndex :
    	        state->selectedIndex - state->pageSize;

	    break;
	case JUMP_DOWN:
    	    jumpValue = (state->pageSize / 2);

    	    state->selectedIndex = (state->selectedIndex + jumpValue) <= state->pageEndIdx ?
    	        state->selectedIndex + jumpValue :
    	        state->pageEndIdx;

	    break;
	case JUMP_UP:
	    jumpValue = (state->pageSize / 2);

	    state->selectedIndex = (state->selectedIndex - jumpValue) >= state->pageStartIdx ?
		state->selectedIndex - jumpValue :
		state->pageStartIdx;

	    break;
	
	default:
	    return;
    }

    if (dir == LEFT || dir == RIGHT) set_start_end_idx(state);
}

static void _read_input(DisplayItems *di, s8 ch)
{
    switch (ch)
    {
	case 'o':
	    di->optionsVisible = !di->optionsVisible;
	    
	    return;
    	case 'q':
	    SHUTDOWN_FLAG = 1;
	    return;
	default:
	    return;
    }
}
