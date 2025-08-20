#include <ncurses.h>
#include <signal.h>

#include "../../include/prc_list_util.h"
#include "../../include/thread.h"
#include "../../include/startup.h"
#include "../../include/menu.h"
#include "../../include/util.h"
#include "../../include/text.h"

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
static void _read_menu_input(UIData *ui, u8 ch);
static void _swap_windows(UIData *ui, WinPosComparisonFn cmp);

void read_arrange_input(UIData *ui)
{
    WindowData *container = ui->windows[CONTAINER_WIN];
    s8 ch = wgetch(container->window);

    flushinp();

    if (ui->menu->isVisible)
    {
	_read_menu_input(ui, ch);
	return;
    }

    if (ch == ('j' & 0x1F)) // ctrl+j
    {
	_swap_windows(ui, win_compare_below);

	return;
    }
    else if (ch == ('k' & 0x1F)) // ctrl+k
    {
	_swap_windows(ui, win_compare_above);

	return;
    }
    else if (ch == ('h' & 0x1F)) // ctrl+h
    {
	_swap_windows(ui, win_compare_left);

	return;
    }
    else if (ch == ('l' & 0x1F)) // ctrl+l
    {
	_swap_windows(ui, win_compare_right);

	return;
    }

    switch (ch)
    {
	case 'q':
	    ui->mode = NORMAL;
	    ui->selectedWindow = false;
	    
	    if (ui->menu->isVisible) ui->menu->isVisible = false;
	    return;
	case 'j':
	    ui->selectedWindow = get_selected_window(ui, win_compare_below);

	    return;
	case 'k':
	    ui->selectedWindow = get_selected_window(ui, win_compare_above);

	    return;
	case 'l':
	    ui->selectedWindow = get_selected_window(ui, win_compare_right);

	    return;
	case 'h':
	    ui->selectedWindow = get_selected_window(ui, win_compare_left);

	    return;
	case 'd':
	    remove_win(ui, ui->selectedWindow);

	    return;
	case 'u':
	    void (*onSelect)(UIData *, MenuItemValue);
	    void (*items)(MenuItem **);
	    const char *title;
	    u8 itemCount;

	    if (mtopSettings->activeWindowCount == 3)
	    {
		onSelect = handle_change_layout;
		items = init_layout_menu_items;
		title = text(TXT_CHOOSE_LAYOUT);
		itemCount = LAYOUT_COUNT;
	    }
	    else if (mtopSettings->activeWindowCount == 2)
	    {
		onSelect = handle_change_duo_orientation;
		items = init_orienation_menu_items;
		title = text(TXT_CHOOSE_ORIENTATION);
		itemCount = ORIENTATION_COUNT;
	    }
	    else return;

	    init_menu(ui, !ui->menu->isVisible, itemCount, title, onSelect, items);

	    return;
	case 'a':
	    u8 isVisible = mtopSettings->activeWindowCount < STAT_WIN_COUNT;

	    init_menu(ui, isVisible, STAT_WIN_COUNT,
	       text(TXT_ADD_WINDOW), handle_add_window, init_stat_menu_items);

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
	    state->infoVisible = false;

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
	state->timeoutActive = false;
    
	return;
    }
    else if (ch == 'd')
    {
	state->cmdBuffer = '\0';
	state->timeoutActive = false;

	kill(state->selectedPid, SIGKILL);

	return;
    }
}

static void _read_menu_input(UIData *ui, u8 ch)
{
    switch (ch)
    {
	case 'j':
	    select_next_menu_item(ui->menu->items, ui->menu->menuItemCount);

	    break;
	case 'k':
	    select_previous_menu_item(ui->menu->items, ui->menu->menuItemCount);

	    break;
	case 'a':
	    ui->menu->isVisible = false;

	    reset_menu_idx(ui->menu->items, STAT_WIN_COUNT);
	    
	    break;
	case 'u':
	    ui->menu->isVisible = false;

	    reset_menu_idx(ui->menu->items, LAYOUT_COUNT);

	    break;
	case 10:
	    // create a function pointer on UIData that we can set
	    // when a menu is opened.
	    MenuItemValue selection = get_menu_selection(ui->menu->items, ui->menu->menuItemCount);

	    ui->menu->on_select(ui, selection);
	    
	    break;
	default:
	    break;
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

static void _swap_windows(UIData *ui, WinPosComparisonFn cmp)
{
    mt_Window windowToSwap = get_selected_window(ui, cmp);
    
    if (ui->selectedWindow == windowToSwap) return;
    
    swap_windows(ui, windowToSwap);
}
