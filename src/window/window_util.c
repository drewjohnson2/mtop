#include <ncurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "../../include/window.h"
#include "../../include/text.h"
#include "../../include/startup.h"

#define S32_MAX 2147483647

static void _reinit_window(UIData * di);

u8 RESIZE = false;

void print_header(const WindowData *wd)
{
    char *user = getlogin();
    
    wattron(wd->window, A_BOLD);
    PRINTFC(wd->window, 0, 2, "%s", text(TXT_MTOP), MT_PAIR_MTOP_LBL);
    wattroff(wd->window, A_BOLD);
    PRINTFC(wd->window, 0, 7, "for %s", user, MT_PAIR_USR_LBL);
}

void print_time(const WindowData *wd)
{
    char timeBuf[10];
    time_t now = time(0);
    struct tm tmNow;

    localtime_r(&now, &tmNow);

    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tmNow);

    PRINTFC(wd->window, 0, wd->wWidth - 10, "%s", timeBuf, MT_PAIR_TM);
}

// Need to move the data calls from this function
// and ^ that function to the IO thread if possible.
// I know it's not a big deal, but that's something
// that I should probably be doing just to be consistent.
void print_uptime_ldAvg(const WindowData *wd)
{
    struct sysinfo info;
    u16 days;
    u64 uptime;
    u16 hours;
    u16 minutes;
    u16 seconds;
    s8 error = sysinfo(&info);

    if (error) return;

    char displayStr[66];

    uptime = info.uptime;
    days = uptime / 86400;
    uptime %= 86400;
    hours = uptime / 3600;
    uptime %= 3600;
    minutes = uptime / 60;
    seconds = uptime %= 60;

    double load[3];
    error = getloadavg(load, 3);

    if (error == -1) return;

    snprintf(
	displayStr,
	sizeof(displayStr),
	text(TXT_UPTIME_LOAD_FMT),
	days,
	hours,
	minutes,
	seconds,
	load[0],
	load[1],
	load[2]
    );

    if (strlen(displayStr) > (wd->wWidth / 2)) return;

    const u8 uptimeX = (wd->wWidth / 2) - (strlen(displayStr) / 2);

    PRINTFC(wd->window, 0, uptimeX, "%s", displayStr, MT_PAIR_TM);
}

void print_footer(const WindowData *wd)
{
    const u8 githubText = wd->wWidth - 30;

    if (!mtopSettings->activeWindows[PRC_WIN])
    {
	PRINTFC(wd->window, wd->wHeight - 1, githubText, "%s", 
	    text(TXT_GITHUB), MT_PAIR_GITHUB);

	return;
    }

    const u8 killPrcCtrlX = 2;
    const u8 killPrcLabelX = killPrcCtrlX + 2;
    const u8 downCtrlX = 19;
    const u8 downLableX = downCtrlX + 2;
    const u8 upCtrlX = 27;
    const u8 upLabelX = upCtrlX + 2;
    const u8 pLeftCtrlX = 33;
    const u8 pLeftLabelX = pLeftCtrlX + 2;
    const u8 pRightCtrlX = 46;
    const u8 pRightLabelX = pRightCtrlX + 2;
    const u8 optionsCtrlX = 60;
    const u8 optionsLabelX = optionsCtrlX + 2;

    PRINTFC(wd->window, wd->wHeight - 1, killPrcCtrlX, "%s", text(TXT_KILL_CTRL), MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, killPrcLabelX, " %s ",
	    text(TXT_KILL), MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, downCtrlX, "%s", text(TXT_DOWN_CTRL), MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, downLableX, "%s", text(TXT_DOWN), MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, upCtrlX, "%s", text(TXT_UP_CTRL), MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, upLabelX, "%s", text(TXT_UP), MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, pLeftCtrlX, "%s", text(TXT_PAGE_LEFT_CTRL), MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, pLeftLabelX, "%s", text(TXT_PAGE_LEFT), MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, pRightCtrlX, "%s", text(TXT_PAGE_RIGHT_CTRL), MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, pRightLabelX, "%s", text(TXT_PAGE_RIGHT), MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, optionsCtrlX, "%s", text(TXT_OPT_CTRL), MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, optionsLabelX, "%s", text(TXT_OPT), MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, githubText, "%s", text(TXT_GITHUB), MT_PAIR_GITHUB);
}

void display_options(UIData *ui)
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
    const u8 sCloseOpts = sMemY + 2;

    werase(ui->windows[OPT_WIN]->window);
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
    PRINTFC(optWin->window, sCloseOpts, ctrlStartX, "%s", text(TXT_OPT_CTRL), MT_PAIR_CTRL);
    PRINTFC(optWin->window, sCloseOpts, infoStartX, "%s ", text(TXT_OPT_CLOSE), 
	    MT_PAIR_CTRL_TXT);
}

void display_stat_types(UIData *ui)
{
    WindowData *statTypeWin = ui->windows[STAT_TYPE_WIN];
    u8 titlePosY = 0;
    const u8 titlePosX = (statTypeWin->wWidth / 2) - (strlen(text(TXT_ADD_WINDOW)) / 2);
    const u8 numPosX = 4;
    const u8 valPosX = 7;
    size_t optionNumber = 1;

    werase(statTypeWin->window);
    SET_COLOR(statTypeWin->window, MT_PAIR_BOX);
    box(statTypeWin->window, 0, 0);
    PRINTFC(statTypeWin->window, titlePosY++, titlePosX, "%s", text(TXT_ADD_WINDOW), MT_PAIR_CTRL_TXT);

    for (size_t i = 0; i < STAT_WIN_COUNT; i++)
    {
	AddWindowMenuItem *item = ui->items[i];
	MT_Color_Pairs pair = MT_PAIR_PRC_UNSEL_TEXT;

	if (mtopSettings->activeWindows[item->returnValue.windowType]) continue;

	if (item->isSelected)
	{
	    pair = MT_PAIR_PRC_SEL_TEXT;

	    for (size_t y = valPosX - 1; y < (size_t)(statTypeWin->wWidth - 4); y++)
		PRINTFC(statTypeWin->window, titlePosY, y, "%c", ' ', pair);
	}

	PRINTFC(statTypeWin->window, titlePosY, numPosX, "%zu.", optionNumber++, MT_PAIR_CPU_HEADER);
	PRINTFC(statTypeWin->window, titlePosY++, valPosX, "%s", item->displayString, pair);
    }
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

void resize_win(UIData *ui)
{
    WindowData *container = ui->windows[CONTAINER_WIN];

    endwin();
    wrefresh(container->window);
    werase(container->window);
    resize_term(0, 0);
    getmaxyx(stdscr, container->wHeight, container->wWidth);
    init_window_dimens(ui);
    wresize(container->window, container->wHeight, container->wWidth);

    _reinit_window(ui);
    
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
    init_window_dimens(ui);
    _reinit_window(ui); 
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
    _reinit_window(ui);
}

void init_stat_menu_items(AddWindowMenuItem **items)
{
    items[0]->displayString = text(TXT_CPU);
    items[0]->returnValue.windowType = CPU_WIN;
    items[0]->isSelected = false;
    items[1]->displayString = text(TXT_MEM);
    items[1]->returnValue.windowType = MEMORY_WIN;
    items[1]->isSelected = false;
    items[2]->displayString = text(TXT_PRC);
    items[2]->returnValue.windowType = PRC_WIN;
    items[2]->isSelected = false;
}

void init_menu_idx(AddWindowMenuItem **items, InitMenuIdxCondFn predicate, u8 itemCount)
{
    for (size_t i = 0; i < itemCount; i++)
    {
	AddWindowMenuItem *item = items[i];

	if (predicate(item))
	{
	    item->isSelected = true;
	    return;
	}
    }
}

void reset_menu_idx(AddWindowMenuItem **items, u8 itemCount)
{
    for (size_t i = 0; i < itemCount; i++) items[i]->isSelected = false;
}

void toggle_add_win_opts(AddWindowMenuItem **items, u8 winCount)
{
    if (mtopSettings->activeWindowCount > 1) return;
    
    AddWindowMenuItem *selectedItem = NULL;
    
    for (size_t i = 0; i < winCount; i++)
    {
        if (items[i]->isSelected)
        {
            selectedItem = items[i];
            break;
        }
    }
    
    for (size_t i = 0; i < winCount; i++)
    {
        if (items[i] == selectedItem) continue;
        else if (mtopSettings->activeWindows[items[i]->returnValue.windowType]) continue;
    
        items[i]->isSelected = true;
        selectedItem->isSelected = false;
    }
}

mt_Window get_add_menu_selection(AddWindowMenuItem **items)
{
    for (size_t i = 0; i < STAT_WIN_COUNT; i++)
    {
	if (items[i]->isSelected)
	{
	    items[i]->isSelected = false;

	    return items[i]->returnValue.windowType;
	}
    }

    return WINDOW_ID_MAX;
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
    _reinit_window(ui);
}

static void _reinit_window(UIData *ui)
{
    u8 winCount = mtopSettings->activeWindowCount;
    mt_Window winType = ui->windowOrder[0];
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *optWin = ui->windows[OPT_WIN];
    WindowData *statTypeWin = ui->windows[STAT_TYPE_WIN];

    for (size_t i = 0; (winType != WINDOW_ID_MAX) && (i < winCount);)
    {
	WindowData *win = ui->windows[winType];

	win->window = subwin(container->window, win->wHeight, win->wWidth, win->windowY, win->windowX);
	winType = ui->windowOrder[++i];
    }

    optWin->window = subwin(container->window, optWin->wHeight, optWin->wWidth, optWin->windowY, optWin->windowX);
    statTypeWin->window = subwin(container->window, statTypeWin->wHeight, statTypeWin->wWidth, statTypeWin->windowY, statTypeWin->windowX);

    REFRESH_WIN(container->window);

    print_header(container);
    print_footer(container);
}
