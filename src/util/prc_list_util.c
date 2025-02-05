
#include <signal.h>

#include "../include/prc_list_util.h"
#include "../include/thread.h"

typedef enum _nav_direction
{
    UP,
    DOWN,
    RIGHT,
    LEFT,
    JUMP_DOWN,
    JUMP_UP
} NavDirection;


void _adjust_menu_index(NavDirection dir, ProcessListState *state);

void set_start_end_idx(ProcessListState *state) 
{
    s8 isLastPage = state->activePage == state->totalPages - 1;
    size_t lastPageEnd = state->maxIndex;
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

void adjust_state(ProcessListState *state, ProcessStats *stats)
{
    // this or set_start_end_idx is messed up
    if (state->maxIndex == (s8)stats->count - 1) return;
    
    state->maxIndex = stats->count - 1;

    u8 updatedPageCount = (state->maxIndex + state->pageSize - 1) / state->pageSize;

    state->totalPages = updatedPageCount < state->totalPages ?
	updatedPageCount :
	state->totalPages;

    set_start_end_idx(state);

    state->selectedIndex = state->selectedIndex > state->maxIndex ?
	state->maxIndex :
	state->selectedIndex;
}

void read_input(
    WINDOW *win,
    ProcessListState *state,
    DisplayItems *di,
    ProcessStatsViewData **vd
)
{
    char ch = wgetch(win);

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
	    state->sortFunc = vd_name_compare_func;

	    return;
	case 'p':
	    if (state->sortOrder == PID && sortDirection == ASC) sortDirection = DESC;
	    else sortDirection = ASC;

	    state->sortOrder = PID;
	    state->sortFunc = vd_pid_compare_func;

	    return;
	case 'c':
	    if (state->sortOrder == CPU && sortDirection == DESC) sortDirection = ASC;
	    else sortDirection = DESC;

	    state->sortOrder = CPU;
	    state->sortFunc = vd_cpu_compare_func;

	    return;
	case 'm':
	    if (state->sortOrder == MEM && sortDirection == DESC) sortDirection = ASC;
	    else sortDirection = DESC;

	    state->sortOrder = MEM;
	    state->sortFunc = vd_mem_compare_func;

	    return;
	case 'o':
	    di->optionsVisible = !di->optionsVisible;
	    
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

    switch (ch)
    {
	case 'd':
	    state->cmdBuffer = '\0';
	    state->timeoutActive = 0;

	    kill(vd[state->selectedIndex]->pid, SIGKILL);

	    return;
	default:
	    break;
    }
}

void _adjust_menu_index(NavDirection dir, ProcessListState *state)
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
