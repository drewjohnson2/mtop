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

static void _adjust_menu_index(NavDirection dir, ProcessListState *state);
static void _read_add_win_menu_input(UIData *ui, u8 ch);
static void _swap_windows(UIData *ui, WinPosComparisonFn cmp);
static u8 _compare_above(WindowData *cmp, WindowData *cur);
static u8 _compare_below(WindowData *cmp, WindowData *cur);
static u8 _compare_left(WindowData *cmp, WindowData *cur);
static u8 _compare_right(WindowData *cmp, WindowData *cur);
static u8 _init_windowType_idx(AddWindowMenuItem *item);

void read_arrange_input(UIData *ui)
{
    WindowData *container = ui->windows[CONTAINER_WIN];
    s8 ch = wgetch(container->window);

    flushinp();

    if (ui->statTypesVisible)
    {
	_read_add_win_menu_input(ui, ch);
	return;
    }

    if (ch == ('j' & 0x1F)) // ctrl+j
    {
	_swap_windows(ui, _compare_below);

	return;
    }
    else if (ch == ('k' & 0x1F)) // ctrl+k
    {
	_swap_windows(ui, _compare_above);

	return;
    }
    else if (ch == ('h' & 0x1F)) // ctrl+h
    {
	_swap_windows(ui, _compare_left);

	return;
    }
    else if (ch == ('l' & 0x1F)) // ctrl+l
    {
	_swap_windows(ui, _compare_right);

	return;
    }

    switch (ch)
    {
	case 'q':
	    ui->mode = NORMAL;
	    ui->selectedWindow = false;
	    
	    if (ui->statTypesVisible) ui->statTypesVisible = 0;
	    return;
	case 'j':
	    ui->selectedWindow = get_selected_window(ui, _compare_below);

	    return;
	case 'k':
	    ui->selectedWindow = get_selected_window(ui, _compare_above);

	    return;
	case 'l':
	    ui->selectedWindow = get_selected_window(ui, _compare_right);

	    return;
	case 'h':
	    ui->selectedWindow = get_selected_window(ui, _compare_left);

	    return;
	case 'd':
	    remove_win(ui, ui->selectedWindow);

	    return;
	case 'a':
	    ui->statTypesVisible = mtopSettings->activeWindowCount < STAT_WIN_COUNT;

	    init_stat_menu_items(ui->items);
	    init_menu_idx(ui->items, _init_windowType_idx, STAT_WIN_COUNT);

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
    UIData *ui
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
	    state->timeoutActive = false;
    	}
    }
    
    if (ch == -1) return;

    if (ch == ('a' & 0x1F)) // ctrl+a
    {
	ui->mode = ARRANGE;
	ui->selectedWindow = ui->windowOrder[0];
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
	    if (mtopSettings->activeWindows[PRC_WIN]) state->infoVisible = true;
	    
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
	    if (!mtopSettings->activeWindows[PRC_WIN]) return;
	    ui->optionsVisible = !ui->optionsVisible;
	    
	    return;
	case 'b':
	    state->infoVisible = 0;

	    return;
    	case 'q':
	    SHUTDOWN_FLAG = true;
	    return;
    	default:
	    break;
    }
    
    if (!state->cmdBuffer)
    {
	state->cmdBuffer = ch;
	state->timeoutActive = true;
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

static void _read_add_win_menu_input(UIData *ui, u8 ch)
{
    switch (ch)
    {
	case 'j':
	    toggle_add_win_opts(ui->items, STAT_WIN_COUNT);

	    break;
	case 'a':
	    ui->statTypesVisible = 0;

	    reset_menu_idx(ui->items, STAT_WIN_COUNT);
	    
	    break;
	case 10:
	    mt_Window winToAdd = get_add_menu_selection(ui->items);

	    if (winToAdd == WINDOW_ID_MAX) return;

	    add_win(ui, winToAdd);

	    ui->statTypesVisible = 0;

	    break;
	default:
	    break;
    }
}

static void _swap_windows(UIData *ui, WinPosComparisonFn cmp)
{
    mt_Window windowToSwap = get_selected_window(ui, cmp);
    
    if (ui->selectedWindow == windowToSwap) return;
    
    swap_windows(ui, windowToSwap);
}

static u8 _compare_above(WindowData *cmp, WindowData *cur)
{
    return cmp->windowY < cur->windowY;
}

static u8 _compare_below(WindowData *cmp, WindowData *cur)
{
    return cmp->windowY > cur->windowY;
}

static u8 _compare_left(WindowData *cmp, WindowData *cur)
{
    return cmp->windowX < cur->windowX;
}

static u8 _compare_right(WindowData *cmp, WindowData *cur)
{
    return cmp->windowX > cur->windowX;
}

static u8 _init_windowType_idx(AddWindowMenuItem *item)
{
    return !mtopSettings->activeWindows[item->returnValue.windowType];
}
