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

#define FULL_WIDTH(container, win) container->wWidth - (win->paddingLeft + win->paddingRight)
#define HALF_WIDTH(container, win) (container->wWidth / 2) - win->paddingRight

#define FULL_HEIGHT(container, win) (container->wHeight) - (win->paddingTop + win->paddingBottom)
#define HALF_HEIGHT(container, win) (container->wHeight / 2) - (win->paddingTop + win->paddingBottom)

#define POS_X_START(win) ((win)->paddingLeft)
#define POS_X_END(container, win) ((container)->wWidth - (win)->paddingRight - (win)->wWidth)

#define POS_Y_START(win) win->paddingTop
#define POS_Y_BOTTOM(conatiner, win) win->wHeight + win->paddingTop

typedef struct _padding_vals
{
    u8 paddingTop;
    u8 paddingBottom;
    u8 paddingLeft;
    u8 paddingRight;
} PaddingValues;

// [orientation][layout][number of wins - 1][position]
static PaddingValues paddingTable[2][6][3][3] = {
    // orientation horizontal
    {
	{},{},
	// layout quarters top
	{ {}, {}, { { 1, 0, 1, 1 }, { 1, 0, 0, 1 }, { 0, 0, 1, 1 } } },
	// layout quarters bottom
	{ {}, {}, { { 1, 0, 1, 1 }, { 0, 0, 1, 1 }, { 0, 0, 0, 1 } } },
	// layout duo
	{ {}, { { 1, 0, 1, 1 }, { 0, 0, 1, 1 }, { 0, 0, 0, 0 } }, {} },
	// layout single
	{ { { 1, 1, 1, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }, {}, {} }
    },
    {
	// layout quarters left
	{ { }, { }, { { 1, 0, 1, 1 }, { 0, 0, 1, 1 }, { 1, 1, 1, 1 } } },
	// layout quarters right
	{ {}, {}, { { 1, 1, 1, 1 }, { 1, 0, 1, 1 }, { 0, 0, 1, 1 } } },
	{},
	{},
	// layout duo
	{ {}, { { 1, 1, 1, 0 }, { 1, 1, 0, 1 }, { 0, 0, 0, 0 } }, {} },
	// layout single
	{ { { 1, 1, 1, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }, {}, {} }

    }
};

static void _setup_opt_win(WindowData *container, WindowData *optWin);
static void _set_padding(
    WindowData *win,
    LayoutOrientation orientation,
    Layout layout,
    u8 winCount,
    u8 position
);

DisplayItems * init_display_items(Arena *arena) 
{
    DisplayItems *di = a_alloc(arena, sizeof(DisplayItems), __alignof(DisplayItems));
    
    assert(di);
    
    di->windowCount = 5;
    di->optionsVisible = 0;
    
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
    getmaxyx(stdscr, wd->wHeight, wd->wWidth);
    noecho();
    curs_set(0);
}

void init_window_dimens(DisplayItems *di, mt_Window selectedWins[3])
{
    LayoutOrientation orientation = mtopSettings->orientation;    
    Layout layout = mtopSettings->layout;
    u8 winCount = mtopSettings->activeWindowCount;
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *winOne = di->windows[selectedWins[0]];
    WindowData *winTwo = winCount >= 2 ? di->windows[selectedWins[1]] : NULL;
    WindowData *winThree = winCount == 3 ? di->windows[selectedWins[2]] : NULL;
    WindowData *optWin = di->windows[OPT_WIN];

    container->windowX = 0;
    container->windowY = 0;

    _set_padding(winOne, orientation, layout, winCount, 0);

    if (winCount >= 2) _set_padding(winTwo, orientation, layout, winCount, 1);
    if (winCount == 3) _set_padding(winThree, orientation, layout, winCount, 2);

    switch (layout) 
    {
	case QUARTERS_LEFT:
	    winOne->wWidth = HALF_WIDTH(container, winOne);
	    winOne->wHeight = HALF_HEIGHT(container, winOne);
	    winOne->windowX = POS_X_START(winOne);
	    winOne->windowY = POS_Y_START(winOne);
	    winOne->windowTitle = _text[selectedWins[0] + 20];

	    winTwo->wWidth = HALF_WIDTH(container, winTwo);
	    winTwo->wHeight = HALF_HEIGHT(container, winTwo);
	    winTwo->windowX = POS_X_START(winTwo);
	    winTwo->windowY = POS_Y_BOTTOM(conatiner, winTwo);
	    winTwo->windowTitle = _text[selectedWins[1] + 20];
	    
	    winThree->wWidth = HALF_WIDTH(container, winThree); 
	    winThree->wHeight = FULL_HEIGHT(container, winThree);
	    winThree->windowX = POS_X_END(container, winThree);
	    winThree->windowY = POS_X_START(winThree);
	    winThree->windowTitle = _text[selectedWins[2] + 20];

	    break;
	case QUARTERS_RIGHT:
	    winOne->wWidth = HALF_WIDTH(container, winOne);
	    winOne->wHeight = FULL_HEIGHT(container, winOne);
	    winOne->windowX = POS_X_START(winOne);
	    winOne->windowY = POS_Y_START(winOne);
	    winOne->windowTitle = _text[selectedWins[0] + 20];
	    
	    winTwo->wWidth = HALF_WIDTH(container, winTwo); 
	    winTwo->wHeight = HALF_HEIGHT(container, winTwo);
	    winTwo->windowX = POS_X_END(container, winTwo);
	    winTwo->windowY = POS_Y_START(winTwo); 
	    winTwo->windowTitle = _text[selectedWins[1] + 20];

	    winThree->wWidth = HALF_WIDTH(container, winThree);
	    winThree->wHeight = HALF_HEIGHT(container, winThree);
	    winThree->windowX = POS_X_END(container, winThree);
	    winThree->windowY = POS_Y_BOTTOM(conatiner, winThree);
	    winThree->windowTitle = _text[selectedWins[2] + 20];
	    
	    break;
	case QUARTERS_TOP:
	    winOne->wWidth = HALF_WIDTH(container, winOne);
	    winOne->wHeight = HALF_HEIGHT(container, winOne);
	    winOne->windowX = POS_X_START(winOne);
	    winOne->windowY = POS_Y_START(winOne);
	    winOne->windowTitle = _text[selectedWins[0] + 20];

	    winTwo->wWidth = HALF_WIDTH(container, winTwo);
	    winTwo->wHeight = HALF_HEIGHT(container, winTwo);
	    winTwo->windowX = POS_X_END(container, winTwo);
	    winTwo->windowY = POS_Y_START(winTwo);
	    winTwo->windowTitle = _text[selectedWins[1] + 20];

	    winThree->wWidth = FULL_WIDTH(container, winThree);
	    winThree->wHeight = HALF_HEIGHT(container, winThree);
	    winThree->windowX = POS_X_START(winThree);
	    winThree->windowY = POS_Y_BOTTOM(container, winThree);
	    winThree->windowTitle = _text[selectedWins[2] + 20];

	    break;
	case QUARTERS_BOTTOM:
	    winOne->wWidth = FULL_WIDTH(container, winOne);
	    winOne->wHeight = HALF_HEIGHT(container, winOne);
	    winOne->windowX = POS_X_START(winOne);
	    winOne->windowY = POS_Y_START(winOne);
	    winOne->windowTitle = _text[selectedWins[0] + 20];

	    winTwo->wWidth = HALF_WIDTH(container, winTwo);
	    winTwo->wHeight = HALF_HEIGHT(container, winTwo);
	    winTwo->windowX = POS_X_START(winTwo);
	    winTwo->windowY = POS_Y_BOTTOM(conatiner, winTwo); 
	    winTwo->windowTitle = _text[selectedWins[1] + 20];

	    winThree->wWidth = HALF_WIDTH(container, winThree);
	    winThree->wHeight = HALF_HEIGHT(container, winThree); 
	    winThree->windowX = POS_X_END(container, winThree);
	    winThree->windowY = POS_Y_BOTTOM(container, winThree);
	    winThree->windowTitle = _text[selectedWins[2] + 20];

	    break;
	case DUO:
	    u8 isHzl = orientation == HORIZONTAL;
	    u8 widthOne = isHzl ? FULL_WIDTH(container, winOne) : HALF_WIDTH(container, winOne);
	    u8 widthTwo = isHzl ? FULL_WIDTH(container, winTwo) : HALF_WIDTH(container, winTwo);
	    u8 heightOne = isHzl ? HALF_HEIGHT(container, winOne) : FULL_HEIGHT(container, winOne);
	    u8 heightTwo = isHzl ? HALF_HEIGHT(container, winTwo) : FULL_HEIGHT(container, winTwo);

	    winOne->wWidth = widthOne;
	    winOne->wHeight = heightOne;
	    winOne->windowX = POS_X_START(winOne);
	    winOne->windowY = POS_Y_START(winOne);
	    winOne->windowTitle = _text[selectedWins[0] + 20];
	                          
	    winTwo->wWidth = widthTwo;
	    winTwo->wHeight = heightTwo;
	    winTwo->windowX = isHzl ? POS_X_START(winTwo) : POS_X_END(container, winTwo);
	    winTwo->windowY = isHzl ? POS_Y_BOTTOM(conatiner, winTwo) : POS_Y_START(winTwo);
	    winTwo->windowTitle = _text[selectedWins[1] + 20];

	    break;

	case SINGLE:
	    winOne->wWidth = FULL_WIDTH(container, winOne);
	    winOne->wHeight = FULL_HEIGHT(container, winOne);
	    winOne->windowX = POS_X_START(winOne);
	    winOne->windowY = POS_Y_START(winOne);

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

    const u8 uptimeX = (wd->wWidth / 2) - (strlen(displayStr) / 2);

    PRINTFC(wd->window, 0, uptimeX, "%s", displayStr, MT_PAIR_TM);
}

void print_footer(const WindowData *wd)
{
    const u8 killPrcCtrlX = 2;
    const u8 killPrcLabelX = killPrcCtrlX + 2;
    const u8 githubText = wd->wWidth - 29;
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
    PRINTFC(wd->window, wd->wHeight - 1, githubText, "%s", 
	_text[2], 
	MT_PAIR_GITHUB);
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

static void _setup_opt_win(WindowData *container, WindowData *optWin)
{
    optWin->wWidth = container->wWidth / 4;
    optWin->wHeight = container->wHeight / 4;

    if (optWin->wHeight < 16) optWin->wHeight = 17;
    if (optWin->wWidth < 50) optWin->wWidth = 55;

    optWin->windowX = (container->wWidth / 2) - (optWin->wWidth / 2);
    optWin->windowY = (container->wHeight / 2) - (optWin->wHeight / 2);
}

static void _set_padding(
    WindowData *win,
    LayoutOrientation orientation,
    Layout layout,
    u8 winCount,
    u8 position
)
{
    win->paddingTop 	= paddingTable[orientation][layout][winCount - 1][position].paddingTop;
    win->paddingBottom 	= paddingTable[orientation][layout][winCount - 1][position].paddingBottom;
    win->paddingLeft 	= paddingTable[orientation][layout][winCount - 1][position].paddingLeft;
    win->paddingRight 	= paddingTable[orientation][layout][winCount - 1][position].paddingRight;
}
