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

static void _setup_opt_win(WindowData *container, WindowData *optWin);

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

void init_window_dimens_full(DisplayItems *di, mt_Window selectedWins[3]) 
{
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *winOne = di->windows[selectedWins[0]];
    WindowData *winTwo = di->windows[selectedWins[1]];
    WindowData *winThree = di->windows[selectedWins[2]];
    WindowData *optWin = di->windows[OPT_WIN];
    
    container->windowX = 0;
    container->windowY = 0;
    
    winOne->paddingTop = 1;
    winOne->paddingBottom = 0;
    winOne->paddingLeft = 1;
    winOne->paddingRight = 1;
    winOne->windowTitle = _text[selectedWins[0] + 20];
    
    winTwo->paddingTop = 1;
    winTwo->paddingBottom = 0;
    winTwo->paddingLeft = 1;
    winTwo->paddingRight = 1;
    winTwo->windowTitle = _text[selectedWins[1] + 20];
    
    winThree->paddingTop = 1;
    winThree->paddingBottom = 0;
    winThree->paddingLeft = 0;
    winThree->paddingRight = 1;
    winThree->windowTitle = _text[selectedWins[2] + 20];
    
    // CPU win
    winOne->wWidth = container->wWidth - (winOne->paddingLeft + winOne->paddingRight);
    winOne->wHeight = (container->wHeight / 2) - (winOne->paddingTop + winOne->paddingBottom);
    winOne->windowX = winOne->paddingLeft;
    winOne->windowY = winOne->paddingTop;
    
    // Memory win
    winTwo->wWidth = (container->wWidth / 2) - winTwo->paddingRight;
    winTwo->wHeight = (container->wHeight / 2) - winTwo->paddingTop;
    winTwo->windowX = winTwo->paddingLeft;
    winTwo->windowY = winOne->wHeight + winTwo->paddingTop;
    
    //Process Win
    winThree->wWidth = winTwo->wWidth;
    winThree->wHeight = (container->wHeight / 2) - winThree->paddingTop; 
    winThree->windowX = container->wWidth - winThree->paddingRight - winThree->wWidth;
    winThree->windowY = winOne->wHeight + winThree->paddingTop;

    _setup_opt_win(container, optWin);
}

// rename this init_window_dimens_duo
void init_window_dimens_duo(DisplayItems *di, mt_Window selectedWins[3])
{
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *selWinOne = di->windows[selectedWins[0]];
    WindowData *selWinTwo = di->windows[selectedWins[1]];
    WindowData *optWin = di->windows[OPT_WIN];

    container->windowX = 0;
    container->windowY = 0;
    
    selWinOne->paddingTop = 1;
    selWinOne->paddingBottom = 0;
    selWinOne->paddingLeft = 1;
    selWinOne->paddingRight = 1;
    selWinOne->windowTitle = _text[selectedWins[0] + 20];

    selWinTwo->paddingTop = 1;
    selWinTwo->paddingBottom = 0;
    selWinTwo->paddingLeft = 1;
    selWinTwo->paddingRight = 1;
    selWinTwo->windowTitle = _text[selectedWins[1] + 20];

    selWinOne->wWidth = container->wWidth - (selWinOne->paddingLeft + selWinOne->paddingRight);
    selWinOne->wHeight = (container->wHeight / 2) - (selWinOne->paddingTop + selWinOne->paddingBottom);
    selWinOne->windowX = selWinOne->paddingLeft;
    selWinOne->windowY = selWinOne->paddingTop;

    selWinTwo->wWidth = container->wWidth - (selWinTwo->paddingLeft + selWinTwo->paddingRight);
    selWinTwo->wHeight = (container->wHeight / 2) - 1;
    selWinTwo->windowX = selWinTwo->paddingLeft;
    selWinTwo->windowY = selWinOne->wHeight + selWinTwo->paddingTop;

    _setup_opt_win(container, optWin);
}


void init_window_dimens_single(DisplayItems *di, mt_Window selectedWin)
{
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *selWin = di->windows[selectedWin];
    WindowData *optWin = di->windows[OPT_WIN];

    container->windowX = 0;
    container->windowY = 0;

    selWin->paddingTop = 1;
    selWin->paddingBottom = 0;
    selWin->paddingLeft = 1;
    selWin->paddingRight = 1;
    selWin->windowTitle = _text[selectedWin + 20];

    selWin->wWidth = container->wWidth - (selWin->paddingLeft + selWin->paddingRight);
    selWin->wHeight = (container->wHeight) - (selWin->paddingTop + selWin->paddingBottom);
    selWin->windowX = selWin->paddingLeft;
    selWin->windowY = selWin->paddingTop;

    _setup_opt_win(container, optWin);
}

void init_window_dimens_vl_full(DisplayItems *di, mt_Window selectedWins[3]) 
{
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *winOne = di->windows[selectedWins[0]];
    WindowData *winTwo = di->windows[selectedWins[1]];
    WindowData *winThree = di->windows[selectedWins[2]];
    WindowData *optWin = di->windows[OPT_WIN];
    
    container->windowX = 0;
    container->windowY = 0;
    
    winOne->paddingTop = 1;
    winOne->paddingBottom = 0;
    winOne->paddingLeft = 1;
    winOne->paddingRight = 1;
    winOne->windowTitle = _text[selectedWins[0] + 20];
    
    winTwo->paddingTop = 1;
    winTwo->paddingBottom = 0;
    winTwo->paddingLeft = 1;
    winTwo->paddingRight = 1;
    winTwo->windowTitle = _text[selectedWins[1] + 20];
    
    winThree->paddingTop = 1;
    winThree->paddingBottom = 2;
    winThree->paddingLeft = 0;
    winThree->paddingRight = 1;
    winThree->windowTitle = _text[selectedWins[2] + 20];
    
    // CPU win
    winOne->wWidth = (container->wWidth / 2) - (winOne->paddingLeft + winOne->paddingRight);
    winOne->wHeight = (container->wHeight / 2) - 1;
    winOne->windowX = winOne->paddingLeft;
    winOne->windowY = winOne->paddingTop;
    
    // Memory win
    winTwo->wWidth = (container->wWidth / 2) - (winTwo->paddingLeft + winTwo->paddingRight);
    winTwo->wHeight = (container->wHeight / 2) - 1;
    winTwo->windowX = winTwo->paddingLeft;
    winTwo->windowY = winOne->wHeight + winTwo->paddingTop;
    
    //Process Win
    winThree->wWidth = (container->wWidth / 2); 
    winThree->wHeight = container->wHeight - (winThree->paddingTop + winThree->paddingBottom); 
    winThree->windowX = container->wWidth - winThree->paddingRight - winThree->wWidth;
    winThree->windowY = winThree->paddingTop;

    _setup_opt_win(container, optWin);
}

void init_window_dimens_vr_full(DisplayItems *di, mt_Window selectedWins[3]) 
{
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *winOne = di->windows[selectedWins[0]];
    WindowData *winTwo = di->windows[selectedWins[1]];
    WindowData *winThree = di->windows[selectedWins[2]];
    WindowData *optWin = di->windows[OPT_WIN];
    
    container->windowX = 0;
    container->windowY = 0;
    
    winOne->paddingTop = 1;
    winOne->paddingBottom = 2;
    winOne->paddingLeft = 1;
    winOne->paddingRight = 1;
    winOne->windowTitle = _text[selectedWins[0] + 20];
    
    winTwo->paddingTop = 1;
    winTwo->paddingBottom = 0;
    winTwo->paddingLeft = 1;
    winTwo->paddingRight = 1;
    winTwo->windowTitle = _text[selectedWins[1] + 20];
    
    winThree->paddingTop = 1;
    winThree->paddingBottom = 2;
    winThree->paddingLeft = 1;
    winThree->paddingRight = 1;
    winThree->windowTitle = _text[selectedWins[2] + 20];
    
    // CPU win
    winOne->wWidth = (container->wWidth / 2);
    winOne->wHeight = container->wHeight - (winOne->paddingTop + winOne->paddingBottom);
    winOne->windowX = winOne->paddingLeft;
    winOne->windowY = winOne->paddingTop;
    
    // Memory win
    winTwo->wWidth = (container->wWidth / 2) - (winTwo->paddingLeft + winTwo->paddingRight);
    winTwo->wHeight = (container->wHeight / 2) - 1;
    winTwo->windowX = container->wWidth - winTwo->paddingRight - winTwo->wWidth;
    winTwo->windowY = winOne->paddingTop;
    
    //Process Win
    winThree->wWidth = (container->wWidth / 2) - (winThree->paddingLeft + winThree->paddingRight); 
    winThree->wHeight = (container->wHeight / 2) - 1; 
    winThree->windowX = container->wWidth - winThree->paddingRight - winThree->wWidth;
    winThree->windowY = winThree->wHeight + winThree->paddingTop;

    _setup_opt_win(container, optWin);
}

void init_window_dimens_v_duo(DisplayItems *di, mt_Window selectedWins[3])
{
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *selWinOne = di->windows[selectedWins[0]];
    WindowData *selWinTwo = di->windows[selectedWins[1]];
    WindowData *optWin = di->windows[OPT_WIN];

    container->windowX = 0;
    container->windowY = 0;
    
    selWinOne->paddingTop = 1;
    selWinOne->paddingBottom = 1;
    selWinOne->paddingLeft = 1;
    selWinOne->paddingRight = 0;
    selWinOne->windowTitle = _text[selectedWins[0] + 20];

    selWinTwo->paddingTop = 1;
    selWinTwo->paddingBottom = 1;
    selWinTwo->paddingLeft = 0;
    selWinTwo->paddingRight = 1;
    selWinTwo->windowTitle = _text[selectedWins[1] + 20];

    selWinOne->wWidth = (container->wWidth / 2) - (selWinOne->paddingLeft + selWinOne->paddingRight);
    selWinOne->wHeight = container->wHeight - (selWinOne->paddingTop + selWinOne->paddingBottom);
    selWinOne->windowX = selWinOne->paddingLeft;
    selWinOne->windowY = selWinOne->paddingTop;

    selWinTwo->wWidth = (container->wWidth / 2) - (selWinTwo->paddingLeft + selWinTwo->paddingRight);
    selWinTwo->wHeight = container->wHeight - (selWinTwo->paddingTop + selWinTwo->paddingBottom);
    selWinTwo->windowX = container->wWidth - selWinTwo->paddingRight - selWinTwo->wWidth;
    selWinTwo->windowY = selWinTwo->paddingTop;

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
