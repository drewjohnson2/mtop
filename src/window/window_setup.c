#include <locale.h>
#include <ncurses.h>
#include <arena.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../../include/window.h"
#include "../../include/mt_colors.h"

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
    start_color();
    raw();
    getmaxyx(stdscr, wd->wHeight, wd->wWidth);
    noecho();
    curs_set(0);
}

void init_window_dimens(DisplayItems *di) 
{
    WindowData *container = di->windows[CONTAINER_WIN];
    WindowData *cpuWin = di->windows[CPU_WIN];
    WindowData *memoryWin = di->windows[MEMORY_WIN];
    WindowData *prcWin = di->windows[PRC_WIN];
    WindowData *optWin = di->windows[OPT_WIN];
    
    container->windowX = 0;
    container->windowY = 0;
    
    cpuWin->paddingTop = 1;
    cpuWin->paddingBottom = 0;
    cpuWin->paddingLeft = 1;
    cpuWin->paddingRight = 1;
    cpuWin->windowTitle = _text[21];
    
    memoryWin->paddingTop = 1;
    memoryWin->paddingBottom = 0;
    memoryWin->paddingLeft = 1;
    memoryWin->paddingRight = 0;
    memoryWin->windowTitle = _text[22];
    
    prcWin->paddingTop = 1;
    prcWin->paddingBottom = 0;
    prcWin->paddingLeft = 1;
    prcWin->paddingRight = 0;
    prcWin->windowTitle = _text[23];
    
    // CPU win
    cpuWin->wWidth = container->wWidth - (cpuWin->paddingLeft + cpuWin->paddingRight);
    cpuWin->wHeight = (container->wHeight / 2) - (cpuWin->paddingTop + cpuWin->paddingBottom);
    cpuWin->windowX = cpuWin->paddingLeft;
    cpuWin->windowY = cpuWin->paddingTop;
    
    // Memory win
    memoryWin->wWidth = (container->wWidth / 2) - (memoryWin->paddingLeft + memoryWin->paddingRight);
    memoryWin->wHeight = (container->wHeight / 2) - 1;
    memoryWin->windowX = memoryWin->paddingLeft;
    memoryWin->windowY = cpuWin->wHeight + memoryWin->paddingTop;
    
    //Process Win
    prcWin->wWidth = (container->wWidth / 2) - (prcWin->paddingLeft + prcWin->paddingRight);
    prcWin->wHeight = (container->wHeight / 2) - 1; 
    prcWin->windowX = memoryWin->wWidth + prcWin->paddingLeft;
    prcWin->windowY = cpuWin->wHeight + prcWin->paddingTop;

    optWin->wWidth = container->wWidth / 4;
    optWin->wHeight = container->wHeight / 4;
    optWin->windowX = (container->wWidth / 2) - (container->wWidth / 8);
    optWin->windowY = (container->wHeight / 2) - (container->wHeight / 8);
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
