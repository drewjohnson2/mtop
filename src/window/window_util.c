#include <ncurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "../../include/window.h"
#include "../../include/text.h"
#include "../../include/startup.h"
#include "../../include/util.h"

#define S32_MAX 2147483647

u8 RESIZE = false;
u8 REINIT = false;

static void _update_orientation(UIData *ui);

void display_normal_options(UIData *ui)
{
    const WindowData *optWin = ui->windows[OPT_WIN];
    const u8 titlePosX = (optWin->wWidth / 2) - (strlen(text(TXT_OPT_WIN_TITLE)) / 2);
    const u8 titlePosY = 0;
    const u8 ctrlStartX = optWin->wWidth / 4;
    const u8 ctrlBtnStartX = ctrlStartX - 2;
    const u8 infoStartX = ctrlStartX + 8;
    const u8 jUpCtrlY = 2;
    const u8 jDownCtrlY = jUpCtrlY + 2;
    const u8 sProcNmY = jDownCtrlY + 2;
    const u8 sPidY = sProcNmY + 2;
    const u8 sCpuY = sPidY + 2;
    const u8 sMemY = sCpuY + 2;
	const u8 jumpTopY = sMemY + 2;
	const u8 jumpBottomY = jumpTopY + 2;
    const u8 closeOpts = jumpBottomY + 2;

    werase(optWin->window);
    SET_COLOR(optWin->window, MT_PAIR_BOX);
    box(optWin->window, 0, 0);

    PRINTFC(optWin->window, titlePosY, titlePosX, "%s", text(TXT_OPT_WIN_TITLE), MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(optWin->window, jUpCtrlY, ctrlBtnStartX, "%s", text(TXT_JUP_CTRL), MT_PAIR_CTRL);
    PRINTFC(optWin->window, jUpCtrlY, infoStartX, "%s", text(TXT_JUP), MT_PAIR_CTRL_TXT);

    PRINTFC(optWin->window, jDownCtrlY, ctrlBtnStartX, "%s", text(TXT_JDOWN_CTRL), MT_PAIR_CTRL);
    PRINTFC(optWin->window, jDownCtrlY, infoStartX, "%s", text(TXT_JDOWN), MT_PAIR_CTRL_TXT);

    PRINTFC(optWin->window, sProcNmY, ctrlStartX, "%s", text(TXT_SORT_PRC_NM_CTRL), MT_PAIR_CTRL);
    PRINTFC(optWin->window, sProcNmY, infoStartX, "%s ", text(TXT_SORT_PRC_NM),
	    MT_PAIR_CTRL_TXT);
    PRINTFC(optWin->window, sPidY, ctrlStartX, "%s", text(TXT_SORT_PID_CTRL), MT_PAIR_CTRL);
    PRINTFC(optWin->window, sPidY, infoStartX, "%s ", text(TXT_SORT_PID), 
	    MT_PAIR_CTRL_TXT);
    PRINTFC(optWin->window, sCpuY, ctrlStartX, "%s", text(TXT_SORT_CPU_CTRL), MT_PAIR_CTRL);
    PRINTFC(optWin->window, sCpuY, infoStartX, "%s ", text(TXT_SORT_CPU), 
	    MT_PAIR_CTRL_TXT);
    PRINTFC(optWin->window, sMemY, ctrlStartX, "%s", text(TXT_SORT_MEM_CTRL), MT_PAIR_CTRL);
    PRINTFC(optWin->window, sMemY, infoStartX, "%s ", text(TXT_SORT_MEM), 
	    MT_PAIR_CTRL_TXT);
	PRINTFC(optWin->window, jumpTopY, ctrlStartX, "%s", text(TXT_JMP_TOP_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, jumpTopY, infoStartX, "%s", text(TXT_JMP_TOP), MT_PAIR_CTRL_TXT);
	PRINTFC(optWin->window, jumpBottomY, ctrlStartX, "%s", text(TXT_JMP_BTM_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, jumpBottomY, infoStartX, "%s", text(TXT_JMP_BTM), MT_PAIR_CTRL_TXT);
    PRINTFC(optWin->window, closeOpts, ctrlStartX, "%s", text(TXT_OPT_CTRL), MT_PAIR_CTRL);
    PRINTFC(optWin->window, closeOpts, infoStartX, "%s ", text(TXT_OPT_CLOSE), MT_PAIR_CTRL_TXT);


}

void display_arrange_options(UIData *ui)
{
    const WindowData *optWin = ui->windows[OPT_WIN];
    const u8 titlePosX = (optWin->wWidth / 2) - (strlen(text(TXT_OPT_WIN_TITLE)) / 2);
    const u8 titlePosY = 0;
	const u8 ctrlStartX = (optWin->wWidth / 4) - 2;
	const u8 ctrlMultStartX = ctrlStartX - 3;
    const u8 ctrlSelStartX = ctrlStartX - 6;
    const u8 infoStartX = ctrlStartX + 9;
	const u8 selectWinY = 4;
	const u8 moveWinY = selectWinY + 2;
	const u8 addWinY = moveWinY + 2;
	const u8 removeWinY = addWinY + 2;
	const u8 layoutChangeY = removeWinY + 2;
	const u8 closeOptsY = layoutChangeY +2;

    werase(optWin->window);
    SET_COLOR(optWin->window, MT_PAIR_BOX);
    box(optWin->window, 0, 0);

	PRINTFC(optWin->window, titlePosY, titlePosX, "%s", text(TXT_OPT_WIN_TITLE), MT_PAIR_PRC_UNSEL_TEXT);
	PRINTFC(optWin->window, selectWinY, ctrlMultStartX, "%s", text(TXT_LEFT_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, selectWinY, ctrlMultStartX + 1, "%s", ",", MT_PAIR_CTRL_TXT);
	PRINTFC(optWin->window, selectWinY, ctrlMultStartX + 2, "%s", text(TXT_DOWN_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, selectWinY, ctrlMultStartX + 3, "%s", ",", MT_PAIR_CTRL_TXT);
	PRINTFC(optWin->window, selectWinY, ctrlMultStartX + 4, "%s", text(TXT_UP_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, selectWinY, ctrlMultStartX + 5, "%s", ",", MT_PAIR_CTRL_TXT);
	PRINTFC(optWin->window, selectWinY, ctrlMultStartX + 6, "%s", text(TXT_RIGHT_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, selectWinY, infoStartX, "%s", "Change Window Selection", MT_PAIR_CTRL_TXT);

	PRINTFC(optWin->window, moveWinY, ctrlSelStartX, "%s", "Ctrl+", MT_PAIR_CTRL);
	PRINTFC(optWin->window, moveWinY, ctrlSelStartX + 5, "%s", text(TXT_LEFT_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, moveWinY, ctrlSelStartX + 6, "%s", ",", MT_PAIR_CTRL_TXT);
	PRINTFC(optWin->window, moveWinY, ctrlSelStartX + 7, "%s", text(TXT_DOWN_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, moveWinY, ctrlSelStartX + 8, "%s", ",", MT_PAIR_CTRL_TXT);
	PRINTFC(optWin->window, moveWinY, ctrlSelStartX + 9, "%s", text(TXT_UP_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, moveWinY, ctrlSelStartX + 10, "%s", ",", MT_PAIR_CTRL_TXT);
	PRINTFC(optWin->window, moveWinY, ctrlSelStartX + 11, "%s", text(TXT_RIGHT_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, moveWinY, infoStartX, "%s", "Move Window", MT_PAIR_CTRL_TXT);

	PRINTFC(optWin->window, addWinY, ctrlStartX, "%s", text(TXT_ADD_WIN_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, addWinY, infoStartX, "%s", text(TXT_ADD_WIN), MT_PAIR_CTRL_TXT);

	PRINTFC(optWin->window, removeWinY, ctrlStartX, "%s", text(TXT_REMOVE_WIN_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, removeWinY, infoStartX, "%s", text(TXT_RMV_STAT_WIN), MT_PAIR_CTRL_TXT);

	PRINTFC(optWin->window, layoutChangeY, ctrlStartX, "%s", text(TXT_CHANGE_LAYOUT_CTRL), MT_PAIR_CTRL);
	PRINTFC(optWin->window, layoutChangeY, infoStartX, "%s", text(TXT_CHANGE_LAYOUT),
		 MT_PAIR_CTRL_TXT);

    PRINTFC(optWin->window, closeOptsY, ctrlStartX, "%s", text(TXT_OPT_CTRL), MT_PAIR_CTRL);
    PRINTFC(optWin->window, closeOptsY, infoStartX, "%s ", text(TXT_OPT_CLOSE), 
	    MT_PAIR_CTRL_TXT);

}

// Maybe use some macro expansion to clean this up.
void set_bg_colors(
    WINDOW *container,
    WINDOW *cpuWin,
    WINDOW *memWin,
    WINDOW *prcWin,
    WINDOW *optWin,
    WINDOW *statTypeWin
)
{
    volatile u8 *activeWindows = mtopSettings->activeWindows;

    wbkgd(container, COLOR_PAIR(MT_PAIR_BACKGROUND));

    if (activeWindows[CPU_WIN]) wbkgd(cpuWin, COLOR_PAIR(MT_PAIR_BACKGROUND));
    if (activeWindows[MEMORY_WIN]) wbkgd(memWin, COLOR_PAIR(MT_PAIR_BACKGROUND));
    if (activeWindows[PRC_WIN]) wbkgd(prcWin, COLOR_PAIR(MT_PAIR_BACKGROUND));

    wbkgd(optWin, COLOR_PAIR(MT_PAIR_BACKGROUND));
    wbkgd(statTypeWin, COLOR_PAIR(MT_PAIR_BACKGROUND));
}

void resize_windows(UIData *ui)
{
    WindowData *container = ui->windows[CONTAINER_WIN];

    endwin();
    wrefresh(container->window);
    werase(container->window);
    resize_term(0, 0);
    getmaxyx(stdscr, container->wHeight, container->wWidth);
    init_window_dimens(ui);
    wresize(container->window, container->wHeight, container->wWidth);

    reinit_window(ui);
    
    RESIZE = false;
}

void remove_win(UIData *ui, mt_Window winToRemove)
{
    if (mtopSettings->activeWindowCount == 1) return;

    mtopSettings->activeWindowCount--;
    mtopSettings->activeWindows[winToRemove] = false;

    ui->windows[winToRemove]->active = false;

    size_t i, j = 0;

    for (i = 0; i < STAT_WIN_COUNT; i++)
    {
		if (ui->windowOrder[i] == winToRemove) ui->windowOrder[i] = WINDOW_ID_MAX;
		if (ui->windowOrder[i] != WINDOW_ID_MAX) ui->windowOrder[j++] = ui->windowOrder[i];
    }

    while(j < STAT_WIN_COUNT) ui->windowOrder[j++] = WINDOW_ID_MAX;

    u8 winCount = mtopSettings->activeWindowCount;

    if (winCount == 2) mtopSettings->layout = DUO;
    else if (winCount == 1) mtopSettings->layout = SINGLE;

    ui->selectedWindow = ui->windowOrder[0];

    wclear(ui->windows[CONTAINER_WIN]->window);

	_update_orientation(ui);
    init_window_dimens(ui);
    reinit_window(ui); 
}

void add_win(UIData *ui, mt_Window winToAdd)
{
    if (mtopSettings->activeWindowCount == 3) return;

    u8 winCount = ++mtopSettings->activeWindowCount;
    mtopSettings->activeWindows[winToAdd] = true;

    for (size_t i = 0; i < STAT_WIN_COUNT; i++)
    {
	if (ui->windowOrder[i] == WINDOW_ID_MAX)
	{
	    ui->windowOrder[i] = winToAdd;
	    break;
	}
    }

    if (winCount == 2) mtopSettings->layout = DUO;
    else if (winCount == 3) 
    {
		mtopSettings->layout = mtopSettings->orientation == HORIZONTAL ?
		    QUARTERS_TOP :
		    QUARTERS_LEFT;
    }

    ui->windows[winToAdd]->active = true;

    wclear(ui->windows[CONTAINER_WIN]->window);
    init_window_dimens(ui);
    reinit_window(ui);
}

mt_Window get_selected_window(UIData *ui, WinPosComparisonFn cmp)
{
    mt_Window windows[3] = { CPU_WIN, MEMORY_WIN, PRC_WIN };
    mt_Window current = ui->selectedWindow;
    mt_Window selectedWindow = current;
    WindowData *cur = ui->windows[current];

    s32 best = S32_MAX;

    for (size_t i = 0; i < STAT_WIN_COUNT; i++)
    {
		WindowData *win = ui->windows[windows[i]];

		if (win == cur || win == NULL) continue;
		else if (!(cmp(win, cur) && win->active)) continue;

		s16 dx = abs(win->windowX - cur->windowX);
		s16 dy = abs(win->windowY - cur->windowY);
		s32 distance = dx + dy;

		if (distance < best)
		{
		    selectedWindow = windows[i];
		    best = distance;
		}
    }

    return selectedWindow;
}

void swap_windows(UIData *ui, mt_Window windowToSwap)
{
    mt_Window current = ui->selectedWindow;
    s8 idxCurrent = -1;
    s8 idxSwap = -1;

    for (size_t i = 0; i < STAT_WIN_COUNT; i++)
    {
		if (ui->windowOrder[i] == current) idxCurrent = i;
		else if (ui->windowOrder[i] == windowToSwap) idxSwap = i;
    }

    if (idxCurrent == -1 || idxSwap == -1) return;

    mt_Window hold = ui->windowOrder[idxCurrent];

    ui->windowOrder[idxCurrent] = ui->windowOrder[idxSwap];
    ui->windowOrder[idxSwap] = hold;

    init_window_dimens(ui);
    reinit_window(ui);
}

void reinit_window(UIData *ui)
{
    u8 winCount = mtopSettings->activeWindowCount;
    mt_Window winType = ui->windowOrder[0];
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *optWin = ui->windows[OPT_WIN];
    WindowData *statTypeWin = ui->windows[STAT_TYPE_WIN];

    for (size_t i = 0; (winType != WINDOW_ID_MAX) && (i < winCount);)
    {
		WindowData *win = ui->windows[winType];

		delwin(win->window);

		win->window = subwin(container->window, win->wHeight, win->wWidth, win->windowY, win->windowX);
		winType = ui->windowOrder[++i];
    }

	delwin(optWin->window);
	delwin(statTypeWin->window);

    optWin->window = subwin(container->window, optWin->wHeight, optWin->wWidth, optWin->windowY, optWin->windowX);
    statTypeWin->window = subwin(container->window, statTypeWin->wHeight, statTypeWin->wWidth, statTypeWin->windowY, statTypeWin->windowX);

    ui->reinitListState = true;
	REINIT = true;

    REFRESH_WIN(container->window);
}

static void _update_orientation(UIData *ui)
{
	for (size_t i = CPU_WIN; i < PRC_WIN + 1; i++)
	{
		if (!ui->windows[i]->active || mtopSettings->activeWindowCount != 2) continue;

		WindowData *current = ui->windows[i];
		mt_Window currentType = i;
		mt_Window result;
		u8 isFullWidth = current->wWidth == ui->windows[CONTAINER_WIN]->wWidth - 2;
		u8 isFullHeight = current->wHeight == ui->windows[CONTAINER_WIN]->wHeight - 2;

		if (current->windowX > 1) result = get_selected_window(ui, win_compare_left);
		else result = get_selected_window(ui, win_compare_right);

		if (result == currentType && !isFullHeight && !isFullWidth)
		{
			mtopSettings->orientation = HORIZONTAL;
			break;
		}

		if (current->windowY > 1) result = get_selected_window(ui, win_compare_above);
		else result = get_selected_window(ui, win_compare_below);

		if (result == currentType && !isFullHeight && !isFullWidth)
		{
			mtopSettings->orientation = VERTICAL;
			break;
		}
	}
}
