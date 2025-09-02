#include <ncurses.h>
#include <signal.h>

#include "../../include/input.h"
#include "../../include/prc_list_util.h"
#include "../../include/thread.h"
#include "../../include/startup.h"
#include "../../include/menu.h"
#include "../../include/util.h"
#include "../../include/text.h"

#define CTRL(ch, key) ch == (key & 0x1F)

enum CommonKeyInput 
{
	OPTIONS = 'o',
	LEFT = 'h',
	DOWN = 'j',
	UP = 'k',
	RIGHT = 'l',
	QUIT = 'q',
	ENTER = 10
};

enum AdjustModeKeyInput
{
	REMOVE = 'd',
	VIEW_LAYOUTS = 'u',
	TOGGLE_ADD = 'a'
};

enum NormalModeKeyInput {
	JUMP_DOWN = 4,
	JUMP_UP = 21,
	SORT_NAME = 'n',
	SORT_PID = 'p',
	SORT_CPU = 'c',
	SORT_MEM = 'm',
	BACK = 'b',
	JUMP_BOTTOM = 'G',
	JUMP_TOP = 'g',
	ARRANGE_MODE = 'a',
	KILL = 'd'
};

static void _adjust_menu_index(u8 dir, ProcessListState *state);
static void _read_menu_input(UIData *ui, u8 ch);
static void _swap_windows(UIData *ui, WinPosComparisonFn cmp);
static void _setup_layout_menu(UIData *ui);

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

    if (CTRL(ch, DOWN)) // ctrl+j
    {
		_swap_windows(ui, u_win_cmp_below);

		return;
    }
    else if (CTRL(ch, UP)) // ctrl+k
    {
		_swap_windows(ui, u_win_cmp_above);

		return;
    }
    else if (CTRL(ch, LEFT)) // ctrl+h
    {
		_swap_windows(ui, u_win_cmp_left);

		return;
    }
    else if (CTRL(ch, RIGHT)) // ctrl+l
    {
		_swap_windows(ui, u_win_cmp_right);

		return;
    }

    switch (ch)
    {
		case QUIT:
		    ui->mode = NORMAL;
		    ui->selectedWindow = false;
		    
		    if (ui->menu->isVisible) ui->menu->isVisible = false;
		    break;
		case DOWN:
		    ui->selectedWindow = get_selected_window(ui, u_win_cmp_below);

		    break;
		case UP:
		    ui->selectedWindow = get_selected_window(ui, u_win_cmp_above);

		    break;
		case RIGHT:
		    ui->selectedWindow = get_selected_window(ui, u_win_cmp_right);

		    break;
		case LEFT:
		    ui->selectedWindow = get_selected_window(ui, u_win_cmp_left);

		    break;
		case REMOVE:
		    remove_win(ui, ui->selectedWindow);

		    break;
		case VIEW_LAYOUTS:
			_setup_layout_menu(ui);

		    break;
		case TOGGLE_ADD:
		    u8 isVisible = mtopSettings->activeWindowCount < STAT_WIN_COUNT;

		    init_menu(ui, isVisible, STAT_WIN_COUNT,
		    	text(TXT_ADD_WINDOW), menu_handle_add_window, init_stat_menu_items);

		    break;
		case OPTIONS:
			ui->optionsVisible = !ui->optionsVisible;
			
			break;
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

    if (CTRL(ch, ARRANGE_MODE)) // ctrl+a
    {
		ui->mode = ARRANGE;
		ui->selectedWindow = ui->windowOrder[0];
		
		return;
    }
    
    switch (ch)
    {
		case DOWN:
		    _adjust_menu_index(DOWN, state);

		    return;
    	case UP:
		    _adjust_menu_index(UP, state);    

		    return;
		case RIGHT:
		    _adjust_menu_index(RIGHT, state);

		    return;
		case LEFT:
		    _adjust_menu_index(LEFT, state);

		    return;
		case JUMP_DOWN: // CTRL + D
		    _adjust_menu_index(JUMP_DOWN, state);
		
		    return;
		case JUMP_UP: // CTRL + U
		    _adjust_menu_index(JUMP_UP, state);

		    return;
		case ENTER: // Enter
		    if (mtopSettings->activeWindows[PRC_WIN]) state->infoVisible = true;
		    
		    return;
		case SORT_NAME:
			sortDirection = util_get_sort_direction(state, PRC_NAME, sortDirection);

		    state->sortOrder = PRC_NAME;
		    state->sortFn = vd_name_compare_fn;

		    return;
		case SORT_PID:
			sortDirection = util_get_sort_direction(state, PID, sortDirection);

		    state->sortOrder = PID;
		    state->sortFn = vd_pid_compare_fn;

		    return;
		case SORT_CPU:
			sortDirection = util_get_sort_direction_num(state, CPU, sortDirection);

		    state->sortOrder = CPU;
		    state->sortFn = vd_cpu_compare_fn;

		    return;
		case SORT_MEM:
			sortDirection = util_get_sort_direction_num(state, MEM, sortDirection);

		    state->sortOrder = MEM;
		    state->sortFn = vd_mem_compare_fn;

		    return;
		case OPTIONS:
		    if (!mtopSettings->activeWindows[PRC_WIN]) return;
		    ui->optionsVisible = !ui->optionsVisible;
		    
		    return;
		case BACK:
		    state->infoVisible = false;

		    return;
		case JUMP_BOTTOM:
			state->selectedIndex = state->pageEndIdx;

			break;
    	case QUIT:
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
	else if (ch == KILL)
	{
		state->cmdBuffer = '\0';
		state->timeoutActive = false;
	
		kill(state->selectedPid, SIGKILL);
	
		return;
	}
	else if (ch == JUMP_TOP)
	{
		state->selectedIndex = state->pageStartIdx;
	}
}

static void _read_menu_input(UIData *ui, u8 ch)
{
    switch (ch)
    {
		case DOWN:
		    menu_select_next_item(ui->menu->items, ui->menu->menuItemCount);
		
		    break;
		case UP:
		    menu_select_previous_item(ui->menu->items, ui->menu->menuItemCount);
		
		    break;
		case TOGGLE_ADD:
		    ui->menu->isVisible = false;
		
		    menu_reset_idx(ui->menu->items, STAT_WIN_COUNT);
		    
		    break;
		case VIEW_LAYOUTS:
		    ui->menu->isVisible = false;
		
		    menu_reset_idx(ui->menu->items, LAYOUT_COUNT);
		
		    break;
		case ENTER:
		    MenuItemValue selection = menu_get_selection(ui->menu->items, ui->menu->menuItemCount);
		
		    ui->menu->on_select(ui, selection);
		    
		    break;
		default:
		    break;
    }
}

// TODO: Change the jump value to something a bit smaller than pageSize / 2.
// maybe pageSize / 4?
static void _adjust_menu_index(u8 dir, ProcessListState *state)
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
	    	jumpValue = (state->pageSize / 4);
	
	    	state->selectedIndex = (state->selectedIndex + jumpValue) <= state->pageEndIdx ?
	    	    state->selectedIndex + jumpValue :
	    	    state->pageEndIdx;
	
		    break;
		case JUMP_UP:
		    jumpValue = (state->pageSize / 4);
	
		    state->selectedIndex = (state->selectedIndex - jumpValue) >= state->pageStartIdx ?
			state->selectedIndex - jumpValue :
			state->pageStartIdx;
	
		    break;
		
		default:
		    return;
    }

    if (dir == LEFT || dir == RIGHT) plu_set_start_end_idx(state);
}

static void _swap_windows(UIData *ui, WinPosComparisonFn cmp)
{
    mt_Window windowToSwap = get_selected_window(ui, cmp);
    
    if (ui->selectedWindow == windowToSwap) return;
    
    swap_windows(ui, windowToSwap);
}

static void _setup_layout_menu(UIData *ui)
{
	void (*onSelect)(UIData *, MenuItemValue);
	void (*items)(MenuItem **);
	const char *title;
	u8 itemCount;
	
	if (mtopSettings->activeWindowCount == 3)
	{
		onSelect = menu_handle_change_layout;
		items = init_layout_menu_items;
		title = text(TXT_CHOOSE_LAYOUT);
		itemCount = LAYOUT_COUNT;
	}
	else if (mtopSettings->activeWindowCount == 2)
	{
		onSelect = menu_handle_change_duo_orientation;
		items = init_orienation_menu_items;
		title = text(TXT_CHOOSE_ORIENTATION);
		itemCount = ORIENTATION_COUNT;
	}
	else return;
	
	init_menu(ui, !ui->menu->isVisible, itemCount, title, onSelect, items);
}
