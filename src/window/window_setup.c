#include <locale.h>
#include <ncurses.h>
#include <arena.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>

#include "../../include/window.h"
#include "../../include/mt_colors.h"
#include "../../include/static_text.h"
#include "../../include/startup.h"

#define FULL_WIDTH(container) ((container)->wWidth - 2)
#define HALF_WIDTH(container) ((FULL_WIDTH(container)) / 2)

#define FULL_HEIGHT(container) ((container)->wHeight - 2)
#define HALF_HEIGHT(container) ((FULL_HEIGHT(container)) / 2)

#define POS_X_START 1
#define POS_X_END(container, win) ((container)->wWidth - (win)->wWidth - 1)

#define POS_Y_START 1
#define POS_Y_BOTTOM(container, win) ((container)->wHeight - (win)->wHeight - 1)

static void _setup_opt_win(WindowData *container, WindowData *optWin);
static void _setup_quarters_left(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    DisplayItems *di,
    const u8 widthEven,
    const u8 heightEven
);
static void _setup_quarters_right(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    DisplayItems *di,
    const u8 widthEven,
    const u8 heightEven
);
static void _setup_quarters_top(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    DisplayItems *di,
    const u8 widthEven,
    const u8 heightEven
);
static void _setup_quarters_bottom(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    DisplayItems *di,
    const u8 widthEven,
    const u8 heightEven
);
static void _setup_duo(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    DisplayItems *di,
    const u8 widthEven,
    const u8 heightEven,
    const LayoutOrientation orientation
);
static void _reinit_window(DisplayItems * di);

u8 RESIZE = 0;

DisplayItems * init_display_items(Arena *arena) 
{
    DisplayItems *di = a_alloc(arena, sizeof(DisplayItems), __alignof(DisplayItems));
    
    assert(di);
    
    di->windowCount = 5;
    di->optionsVisible = 0;
    di->windowOrder[0] = WINDOW_ID_MAX;
    di->windowOrder[1] = WINDOW_ID_MAX;
    di->windowOrder[2] = WINDOW_ID_MAX;
    
    di->windows = a_alloc(
    	arena,
    	sizeof(WindowData *) * di->windowCount,
    	__alignof(WindowData *)
    );
    
    assert(di->windows);
    
    di->windows[CONTAINER_WIN] = a_alloc(
    	arena, 
    	sizeof(WindowData),
    	__alignof(WindowData)
    );

#define DEFINE_WINDOWS(winName, enumName) 	\
    di->windows[enumName] = a_alloc( 		\
	arena, 					\
    	sizeof(WindowData), 			\
    	__alignof(WindowData) 			\
    ); 						\
						\
    assert(di->windows[enumName]);		
#include "../../include/tables/window_def_table.h"
#undef DEFINE_WINDOWS

    return di;
}

void init_ncurses(WindowData *wd, SCREEN *screen)
{
    setlocale(LC_ALL, "");
    set_term(screen);
    use_default_colors();
    start_color();
    raw();
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, wd->wHeight, wd->wWidth);
    noecho();
    curs_set(0);
}

// need to fix full screen duo vertical. It's all messed up for some reason.
void init_window_dimens(DisplayItems *di)
{
    const LayoutOrientation orientation = mtopSettings->orientation;    
    const Layout layout = mtopSettings->layout;
    const u8 winCount = mtopSettings->activeWindowCount;
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *winOne = di->windows[di->windowOrder[0]];
    WindowData *winTwo = winCount >= 2 ? di->windows[di->windowOrder[1]] : NULL;
    WindowData *winThree = winCount == 3 ? di->windows[di->windowOrder[2]] : NULL;
    WindowData *optWin = di->windows[OPT_WIN];
    const u8 widthEven = container->wWidth % 2 == 0;
    const u8 heightEven = container->wHeight % 2 == 0;

    winOne->active = 1;
    if (winTwo) winTwo->active = 1;
    if (winThree) winThree->active = 1;

    container->windowX = 0;
    container->windowY = 0;

    // I don't like this. This is really stupid.
    // Find a better way
    switch (layout) 
    {
	case QUARTERS_LEFT:
	    _setup_quarters_left(
		container,
		winOne,
		winTwo,
		winThree,
		di,
		widthEven,
		heightEven
	    );	   

	    break;
	case QUARTERS_RIGHT:
	    _setup_quarters_right(
		container,
		winOne,
		winTwo,
		winThree,
		di,
		widthEven,
		heightEven
	    );
	    
	    break;
	case QUARTERS_TOP: 
	    _setup_quarters_top(
		container,
		winOne,
		winTwo,
		winThree,
		di,
		widthEven,
		heightEven
	    );

	    break;
	case QUARTERS_BOTTOM:
	    _setup_quarters_bottom(
		container,
		winOne,
		winTwo,
		winThree,
		di,
		widthEven,
		heightEven
	    );

	    break;
	case DUO:
	    _setup_duo(
		container,
		winOne,
		winTwo,
		di,
		widthEven,
		heightEven,
		orientation
	    );

	    break;

	case SINGLE:
	    winOne->wWidth = FULL_WIDTH(container);
	    winOne->wHeight = FULL_HEIGHT(container);
	    winOne->windowX = POS_X_START;
	    winOne->windowY = POS_Y_START;
	    winOne->windowTitle = _text[di->windowOrder[0] + 20];

	    break;
	default:
	    exit(0);
    }

    _setup_opt_win(container, optWin);
}

void init_windows(DisplayItems *di) 
{
    WindowData *container = di->windows[CONTAINER_WIN];
    
     container->window = newwin(
    	container->wHeight,
    	container->wWidth,
    	container->windowY,
    	container->windowX
    );
    
    nodelay(container->window, TRUE);

#define DEFINE_WINDOWS(winName, enumName) 		\
    WindowData *winName##Win = di->windows[enumName]; 	\
							\
    winName##Win->window = subwin(			\
    	container->window,				\
    	winName##Win->wHeight,				\
    	winName##Win->wWidth,				\
    	winName##Win->windowY,				\
    	winName##Win->windowX				\
    );
#include "../../include/tables/window_def_table.h"
#undef DEFINE_WINDOWS

    assert(
    	container->window &&
    	cpuWin->window &&
    	memoryWin->window &&
    	prcWin->window &&
	optWin->window
    );
}

void print_header(const WindowData *wd)
{
    char *user = getlogin();
    
    wattron(wd->window, A_BOLD);
    PRINTFC(wd->window, 0, 2, "%s", _text[20], MT_PAIR_MTOP_LBL);
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
	_text[39],
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
	    _text[2], MT_PAIR_GITHUB);

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

    PRINTFC(wd->window, wd->wHeight - 1, killPrcCtrlX, "%s", _text[0], MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, killPrcLabelX, " %s ",
	    _text[1], MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, downCtrlX, "%s", _text[3], MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, downLableX, "%s", _text[4], MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, upCtrlX, "%s", _text[5], MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, upLabelX, "%s", _text[6], MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, pLeftCtrlX, "%s", _text[29], MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, pLeftLabelX, "%s",
	    _text[28], MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, pRightCtrlX, "%s", _text[31], MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, pRightLabelX, "%s",
	    _text[30], MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, optionsCtrlX, "%s", _text[7], MT_PAIR_CTRL);
    PRINTFC(wd->window, wd->wHeight - 1, optionsLabelX, "%s", _text[8], MT_PAIR_CTRL_TXT);
    PRINTFC(wd->window, wd->wHeight - 1, githubText, "%s", _text[2], MT_PAIR_GITHUB);
}

void display_options(DisplayItems *di)
{
    const WindowData *optWin = di->windows[OPT_WIN];
    const u8 titlePosX = (optWin->wWidth / 2) - (strlen(_text[9]) / 2);
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

    werase(di->windows[OPT_WIN]->window);
    SET_COLOR(optWin->window, MT_PAIR_BOX);
    box(optWin->window, 0, 0);

    PRINTFC(optWin->window, titlePosY, titlePosX, "%s", _text[9], MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(optWin->window, jUpCtrlY, ctrlBtnStartX, "%s", _text[32], MT_PAIR_CTRL);
    PRINTFC(optWin->window, jUpCtrlY, infoStartX, "%s", _text[33], MT_PAIR_CTRL_TXT);

    PRINTFC(optWin->window, jDownCtrlY, ctrlBtnStartX, "%s", _text[34], MT_PAIR_CTRL);
    PRINTFC(optWin->window, jDownCtrlY, infoStartX, "%s", _text[35], MT_PAIR_CTRL_TXT);

    PRINTFC(optWin->window, sProcNmY, ctrlStartX, "%s", _text[10], MT_PAIR_CTRL);
    PRINTFC(optWin->window, sProcNmY, infoStartX, "%s ", _text[11],
	    MT_PAIR_CTRL_TXT);
    PRINTFC(optWin->window, sPidY, ctrlStartX, "%s", _text[12], MT_PAIR_CTRL);
    PRINTFC(optWin->window, sPidY, infoStartX, "%s ", _text[13], 
	    MT_PAIR_CTRL_TXT);
    PRINTFC(optWin->window, sCpuY, ctrlStartX, "%s", _text[14], MT_PAIR_CTRL);
    PRINTFC(optWin->window, sCpuY, infoStartX, "%s ", _text[15], 
	    MT_PAIR_CTRL_TXT);
    PRINTFC(optWin->window, sMemY, ctrlStartX, "%s", _text[16], MT_PAIR_CTRL);
    PRINTFC(optWin->window, sMemY, infoStartX, "%s ", _text[17], 
	    MT_PAIR_CTRL_TXT);
    PRINTFC(optWin->window, sCloseOpts, ctrlStartX, "%s", _text[18], MT_PAIR_CTRL);
    PRINTFC(optWin->window, sCloseOpts, infoStartX, "%s ", _text[19], 
	    MT_PAIR_CTRL_TXT);
}

void set_bg_colors(
    WINDOW *container,
    WINDOW *cpuWin,
    WINDOW *memWin,
    WINDOW *prcWin,
    WINDOW *optWin
)
{
    volatile u8 *activeWindows = mtopSettings->activeWindows;

    wbkgd(container, COLOR_PAIR(MT_PAIR_BACKGROUND));

    if (activeWindows[CPU_WIN]) wbkgd(cpuWin, COLOR_PAIR(MT_PAIR_BACKGROUND));
    if (activeWindows[MEMORY_WIN]) wbkgd(memWin, COLOR_PAIR(MT_PAIR_BACKGROUND));
    if (activeWindows[PRC_WIN]) wbkgd(prcWin, COLOR_PAIR(MT_PAIR_BACKGROUND));

    wbkgd(optWin, COLOR_PAIR(MT_PAIR_BACKGROUND));
}

void resize_win(DisplayItems *di)
{
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *optWin = di->windows[OPT_WIN];

    endwin();
    wrefresh(container->window);
    werase(container->window);
    resize_term(0, 0);
    getmaxyx(stdscr, container->wHeight, container->wWidth);
    init_window_dimens(di);
    wresize(container->window, container->wHeight, container->wWidth);

    _reinit_window(di);
    
    RESIZE = 0;
}

void remove_win(DisplayItems *di, mt_Window winToRemove)
{
    u8 removedIndex = -1;

    mtopSettings->activeWindowCount--;
    mtopSettings->activeWindows[winToRemove] = 0;

    di->windows[winToRemove]->active = 0;

    for (size_t i = 0; i < 3; i++)
    {
	if (di->windowOrder[i] == winToRemove) 
	{
	    di->windowOrder[i] = WINDOW_ID_MAX;
	    removedIndex = i;
	}
    }

    for (size_t i = removedIndex; i < 2; i++)
    {
	di->windowOrder[i] = di->windowOrder[i + 1];
    }

    // Probably need a better way of doing this
    di->windowOrder[2] = WINDOW_ID_MAX;

    u8 winCount = mtopSettings->activeWindowCount;

    if (winCount == 2) mtopSettings->layout = DUO;
    else if (winCount == 1) mtopSettings->layout = SINGLE;

    di->selectedWindow = di->windowOrder[0];

    init_window_dimens(di);
    _reinit_window(di); 
}

static void _setup_opt_win(WindowData *container, WindowData *optWin)
{
    optWin->wWidth = container->wWidth / 4;
    optWin->wHeight = container->wHeight / 4;

    if (optWin->wHeight < 16) optWin->wHeight = 17;
    if (optWin->wWidth < 50) optWin->wWidth = 55;

    optWin->windowX = (container->wWidth / 2) - (optWin->wWidth / 2);
    optWin->windowY = (container->wHeight / 2) - (optWin->wHeight / 2);
}

static void _setup_quarters_left(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    DisplayItems *di,
    const u8 widthEven,
    const u8 heightEven
)
{
    winOne->wWidth = HALF_WIDTH(container);
    winOne->wHeight = HALF_HEIGHT(container);
    winOne->windowX = POS_X_START;
    winOne->windowY = POS_Y_START;
    winOne->windowTitle = _text[di->windowOrder[0] + 20];
    
    winTwo->wWidth = widthEven ?
        HALF_WIDTH(container) :
    	HALF_WIDTH(container) + 1;
    winTwo->wHeight = FULL_HEIGHT(container);
    winTwo->windowX = POS_X_END(container, winTwo);
    winTwo->windowY = POS_X_START;
    winTwo->windowTitle = _text[di->windowOrder[1] + 20];
    
    winThree->wWidth = HALF_WIDTH(container);
    winThree->wHeight = heightEven ? 
        HALF_HEIGHT(container) :
        HALF_HEIGHT(container) + 1;
    winThree->windowX = POS_X_START;
    winThree->windowY = POS_Y_BOTTOM(container, winThree);
    winThree->windowTitle = _text[di->windowOrder[2] + 20];
}

static void _setup_quarters_right(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    DisplayItems *di,
    const u8 widthEven,
    const u8 heightEven
)
{
    winOne->wWidth = widthEven ?
        HALF_WIDTH(container) :
        HALF_WIDTH(container) + 1;
    winOne->wHeight = FULL_HEIGHT(container);
    winOne->windowX = POS_X_START;
    winOne->windowY = POS_Y_START;
    winOne->windowTitle = _text[di->windowOrder[0] + 20];
    
    winTwo->wWidth = HALF_WIDTH(container); 
    winTwo->wHeight = HALF_HEIGHT(container);
    winTwo->windowX = POS_X_END(container, winTwo);
    winTwo->windowY = POS_Y_START; 
    winTwo->windowTitle = _text[di->windowOrder[1] + 20];
    
    winThree->wWidth = HALF_WIDTH(container);
    winThree->wHeight = heightEven ?
        HALF_HEIGHT(container) :
        HALF_HEIGHT(container) + 1;
    winThree->windowX = POS_X_END(container, winThree);
    winThree->windowY = POS_Y_BOTTOM(container, winThree);
    winThree->windowTitle = _text[di->windowOrder[2] + 20];
}

static void _setup_quarters_top(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    DisplayItems *di,
    const u8 widthEven,
    const u8 heightEven
)
{
    winOne->wWidth = HALF_WIDTH(container);
    winOne->wHeight = HALF_HEIGHT(container);
    winOne->windowX = POS_X_START;
    winOne->windowY = POS_Y_START;
    winOne->windowTitle = _text[di->windowOrder[0] + 20];
    
    winTwo->wWidth = widthEven ?
        HALF_WIDTH(container) :
        HALF_WIDTH(container) + 1;
    winTwo->wHeight = HALF_HEIGHT(container);
    winTwo->windowX = POS_X_END(container, winTwo);
    winTwo->windowY = POS_Y_START;
    winTwo->windowTitle = _text[di->windowOrder[1] + 20];
    
    winThree->wWidth = FULL_WIDTH(container);
    winThree->wHeight = heightEven ?
        HALF_HEIGHT(container) :
        HALF_HEIGHT(container) + 1;
    winThree->windowX = POS_X_START;
    winThree->windowY = POS_Y_BOTTOM(container, winThree);
    winThree->windowTitle = _text[di->windowOrder[2] + 20];
}

static void _setup_quarters_bottom(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    DisplayItems *di,
    const u8 widthEven,
    const u8 heightEven
)
{
    winOne->wWidth = FULL_WIDTH(container);
    winOne->wHeight = heightEven ?
        HALF_HEIGHT(container) :
        HALF_HEIGHT(container) + 1;
    winOne->windowX = POS_X_START;
    winOne->windowY = POS_Y_START;
    winOne->windowTitle = _text[di->windowOrder[0] + 20];
    
    winTwo->wWidth = HALF_WIDTH(container);
    winTwo->wHeight = HALF_HEIGHT(container);
    winTwo->windowX = POS_X_START;
    winTwo->windowY = POS_Y_BOTTOM(container, winTwo); 
    winTwo->windowTitle = _text[di->windowOrder[1] + 20];
    
    winThree->wWidth = widthEven ?
        HALF_WIDTH(container) :
        HALF_WIDTH(container) + 1;
    winThree->wHeight = HALF_HEIGHT(container); 
    winThree->windowX = POS_X_END(container, winThree);
    winThree->windowY = POS_Y_BOTTOM(container, winThree);
    winThree->windowTitle = _text[di->windowOrder[2] + 20];
}

static void _setup_duo(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    DisplayItems *di,
    const u8 widthEven,
    const u8 heightEven,
    const LayoutOrientation orientation
)
{
    u8 isHzl = orientation == HORIZONTAL;
    u8 widthOne = isHzl ? FULL_WIDTH(container) : HALF_WIDTH(container);
    u8 widthTwo = isHzl ? FULL_WIDTH(container) : HALF_WIDTH(container);
    u8 heightOne = isHzl ? HALF_HEIGHT(container) : FULL_HEIGHT(container);
    u8 heightTwo = isHzl ? HALF_HEIGHT(container) : FULL_HEIGHT(container);
    
    if (!widthEven && !isHzl) widthOne++;
    if (!heightEven && isHzl) heightOne++;
    
    winOne->wWidth = widthOne;
    winOne->wHeight = heightOne;
    winOne->windowX = POS_X_START;
    winOne->windowY = POS_Y_START;
    winOne->windowTitle = _text[di->windowOrder[0] + 20];
                          
    winTwo->wWidth = widthTwo;
    winTwo->wHeight = heightTwo;
    winTwo->windowX = isHzl ? POS_X_START : POS_X_END(container, winTwo);
    winTwo->windowY = isHzl ? POS_Y_BOTTOM(container, winTwo) : POS_Y_START;
    winTwo->windowTitle = _text[di->windowOrder[1] + 20];
}

static void _reinit_window(DisplayItems * di)
{
    u8 winCount = mtopSettings->activeWindowCount;
    mt_Window winType = di->windowOrder[0];
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *optWin = di->windows[OPT_WIN];

    for (size_t i = 0; (winType != WINDOW_ID_MAX) && (i < winCount);)
    {
	WindowData *win = di->windows[winType];

	win->window = subwin(container->window, win->wHeight, win->wWidth, win->windowY, win->windowX);
	winType = di->windowOrder[++i];
    }

    optWin->window = subwin(container->window, optWin->wHeight, optWin->wWidth, optWin->windowY, optWin->windowX);

    REFRESH_WIN(container->window);

    print_header(container);
    print_footer(container);
}
