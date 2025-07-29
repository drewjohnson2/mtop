#include <ncurses.h>
#include <signal.h>

#include "../../include/prc_list_util.h"
#include "../../include/thread.h"
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

typedef u16 (*compareFn)(WindowData *cmp, WindowData *cur);

static void _adjust_menu_index(NavDirection dir, ProcessListState *state);
static void _read_input(DisplayItems *di, s8 ch);
static mt_Window _get_selected_window(mt_Window current, DisplayItems *di, compareFn cmp);
static u16 _compare_above(WindowData *cmp, WindowData *cur);
static u16 _compare_below(WindowData *cmp, WindowData *cur);
static u16 _compare_left(WindowData *cmp, WindowData *cur);
static u16 _compare_right(WindowData *cmp, WindowData *cur);

void read_arrange_input(DisplayItems *di)
{
    WindowData *container = di->windows[CONTAINER_WIN];
    s8 ch = wgetch(container->window);

    flushinp();

    switch (ch)
    {
	case 'q':
	    di->mode = NORMAL;
	    di->selectedWindow = 0;
	    
	    return;
	case 'j':
	    di->selectedWindow = _get_selected_window(di->selectedWindow, di, _compare_below);

	    return;
	case 'k':
	    di->selectedWindow = _get_selected_window(di->selectedWindow, di, _compare_above);

	    return;
	case 'l':
	    di->selectedWindow = _get_selected_window(di->selectedWindow, di, _compare_right);

	    return;
	case 'h':
	    di->selectedWindow = _get_selected_window(di->selectedWindow, di, _compare_left);

	    return;
	case 'd':
	    remove_win(di, di->selectedWindow);

	    return;
	default:
	    break;
    }
}

// This function is kinda rough.
// I don't like it, but I am not going
// to rewrite it. I'm not even really sure
// how much I could do much to improve it. 
void read_normal_input(
    WINDOW *win,
    ProcessListState *state,
    DisplayItems *di
)
{
    s8 ch = wgetch(win);

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

    if (ch == ('a' & 0x1F)) // ctrl+a
    {
	di->mode = ARRANGE;
	di->selectedWindow = di->windowOrder[0];
	return;
    }
    
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
	case 10: // Enter
	    if (mtopSettings->activeWindows[PRC_WIN]) state->infoVisible = 1;
	    
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
    else if (ch == 'd')
    {
	state->cmdBuffer = '\0';
	state->timeoutActive = 0;

	kill(state->selectedPid, SIGKILL);

	return;
    }
}

static void _adjust_menu_index(NavDirection dir, ProcessListState *state)
{
    if (!mtopSettings->activeWindows[PRC_WIN]) return;

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

static mt_Window _get_selected_window(mt_Window current, DisplayItems *di, compareFn cmp)
{
    mt_Window windows[3] = { CPU_WIN, MEMORY_WIN, PRC_WIN };
    mt_Window selectedWindow = current;
    WindowData *cur = di->windows[current];

    s32 best = 10000000;

    for (size_t i = 0; i < 3; i++)
    {
	WindowData *win = di->windows[windows[i]];

	if (win == cur || win == NULL) continue;

	if (cmp(win, cur) && win->active)
	{
	    int dx = win->windowX - cur->windowX;
	    int dy = win->windowY - cur->windowY;

	    s32 distance = dx * dx + dy * dy;

	    if (distance < best)
	    {
		selectedWindow = windows[i];
		best = distance;
	    }
	}
    }

    return selectedWindow;
}

static u16 _compare_above(WindowData *cmp, WindowData *cur)
{
    return cmp->windowY < cur->windowY;
}

static u16 _compare_below(WindowData *cmp, WindowData *cur)
{
    return cmp->windowY > cur->windowY;
}

static u16 _compare_left(WindowData *cmp, WindowData *cur)
{
    return cmp->windowX < cur->windowX;
}

static u16 _compare_right(WindowData *cmp, WindowData *cur)
{
    return cmp->windowX > cur->windowX;
}
