#include <locale.h>
#include <ncurses.h>
#include <arena.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../include/window.h"
#include "../include/mt_colors.h"

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
#include "../include/tables/window_def_table.h"
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
#include "../include/tables/window_def_table.h"
#undef DEFINE_WINDOWS

    assert(
    	container->window &&
    	cpuWin->window &&
    	memoryWin->window &&
    	prcWin->window &&
	optWin->window
    );
}

void print_header(WindowData *wd)
{
    char *user = getlogin();
    
    wattron(wd->window, A_BOLD);
    PRINTFC(wd->window, 0, 2, "%s", _text[20], MT_PAIR_PRC_UNSEL_TEXT);
    wattroff(wd->window, A_BOLD);
    PRINTFC(wd->window, 0, 7, "for %s", user, MT_PAIR_BOX);
}

void print_time(WindowData *wd)
{
    char timeBuf[10];
    time_t now = time(0);
    struct tm tmNow;

    localtime_r(&now, &tmNow);

    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tmNow);

    PRINTFC(wd->window, 0, wd->wWidth - 10, "%s", timeBuf, MT_PAIR_BOX);
}

void print_footer(WindowData *wd)
{
    PRINTFC(wd->window, wd->wHeight - 1, 2, "%s", _text[0], MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 4, " %s ", _text[1], MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, wd->wWidth - 29, "%s", 
	_text[2], 
	MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 19, "%s", _text[3], MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 21, "%s", _text[4], MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 27, "%s", _text[5], MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 29, "%s", _text[6], MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 33, "%s", _text[7], MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 35, "%s", _text[8],
	MT_PAIR_PRC_UNSEL_TEXT);
}

void display_options(DisplayItems *di)
{
    WindowData *optWin = di->windows[OPT_WIN];
    const u8 titlePos = (optWin->wWidth / 2) - (strlen(_text[9]) / 2);
    const u8 ctrlStartX = optWin->wWidth / 3;
    const u8 infoStartX = ctrlStartX + 2;

    werase(di->windows[OPT_WIN]->window);
    SET_COLOR(optWin->window, MT_PAIR_BOX);
    box(optWin->window, 0, 0);

    PRINTFC(optWin->window, 0, titlePos, "%s", _text[9], MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(optWin->window, 4, ctrlStartX, "%s", _text[10], MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(optWin->window, 4, infoStartX, "%s ", _text[11],
	    MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(optWin->window, 6, ctrlStartX, "%s", _text[12], MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(optWin->window, 6, infoStartX, "%s ", _text[13], 
	    MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(optWin->window, 8, ctrlStartX, "%s", _text[14], MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(optWin->window, 8, infoStartX, "%s ", _text[15], 
	    MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(optWin->window, 10, ctrlStartX, "%s", _text[16], MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(optWin->window, 10, infoStartX, "%s ", _text[17], 
	    MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(optWin->window, 12, ctrlStartX, "%s", _text[18], MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(optWin->window, 12, infoStartX, "%s ", _text[19], 
	    MT_PAIR_PRC_UNSEL_TEXT);


}
