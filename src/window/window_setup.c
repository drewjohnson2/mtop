#include <locale.h>
#include <ncurses.h>
#include <arena.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include "../include/window.h"
#include "../include/mt_colors.h"

DisplayItems * init_display_items(Arena *arena) 
{
    DisplayItems *di = a_alloc(arena, sizeof(DisplayItems), __alignof(DisplayItems));
    
    assert(di);
    
    di->windowCount = 4;
    
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
    
    container->windowX = 0;
    container->windowY = 0;
    
    cpuWin->paddingTop = 2;
    cpuWin->paddingBottom = 0;
    cpuWin->paddingLeft = 1;
    cpuWin->paddingRight = 1;
    cpuWin->windowTitle = "CPU Usage";
    
    memoryWin->paddingTop = 2;
    memoryWin->paddingBottom = 0;
    memoryWin->paddingLeft = 1;
    memoryWin->paddingRight = 0;
    memoryWin->windowTitle = "Memory Usage";
    
    prcWin->paddingTop = 2;
    prcWin->paddingBottom = 0;
    prcWin->paddingLeft = 1;
    prcWin->paddingRight = 0;
    prcWin->windowTitle = "Process List";
    
    // CPU win
    cpuWin->wWidth = container->wWidth - (cpuWin->paddingLeft + cpuWin->paddingRight);
    cpuWin->wHeight = (container->wHeight / 2) - (cpuWin->paddingTop + cpuWin->paddingBottom);
    cpuWin->windowX = cpuWin->paddingLeft;
    cpuWin->windowY = cpuWin->paddingTop;
    
    // Memory win
    memoryWin->wWidth = (container->wWidth / 2) - (memoryWin->paddingLeft + memoryWin->paddingRight);
    memoryWin->wHeight = (container->wHeight / 2) - 2;
    memoryWin->windowX = memoryWin->paddingLeft;
    memoryWin->windowY = cpuWin->wHeight + memoryWin->paddingTop;
    
    //Process Win
    prcWin->wWidth = (container->wWidth / 2) - (prcWin->paddingLeft + prcWin->paddingRight);
    prcWin->wHeight = (container->wHeight / 2) - 2; 
    prcWin->windowX = memoryWin->wWidth + prcWin->paddingLeft;
    prcWin->windowY = cpuWin->wHeight + prcWin->paddingTop;
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
    	prcWin->window
    );
}

void print_header(WindowData *wd)
{
    char *user = getlogin();
    
    wattron(wd->window, A_BOLD);
    PRINTFC(wd->window, 1, 2, "%s", "mtop", MT_PAIR_PRC_UNSEL_TEXT);
    wattroff(wd->window, A_BOLD);
    PRINTFC(wd->window, 1, 7, "for %s", user, MT_PAIR_BOX);
}

void print_time(WindowData *wd)
{
    char timeBuf[10];
    time_t now = time(0);
    struct tm tmNow;

    localtime_r(&now, &tmNow);

    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tmNow);

    PRINTFC(wd->window, 1, wd->wWidth - 10, "%s", timeBuf, MT_PAIR_BOX);
}

void print_footer(WindowData *wd)
{
    PRINTFC(wd->window, wd->wHeight - 1, 2, "%s", "dd", MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 4, "%s", " Kill Process", MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 19, "%s", "nn", MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 21, "%s", " Sort By Process", MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 39, "%s", "pp", MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 41, "%s", " Sort By PID", 
	    MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 55, "%s", "cc", MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 57, "%s", " Sort By CPU Usage", 
	    MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 77, "%s", "mm", MT_PAIR_PRC_SEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, 79, "%s", " Sort By Memory Usage", 
	    MT_PAIR_PRC_UNSEL_TEXT);
    PRINTFC(wd->window, wd->wHeight - 1, wd->wWidth - 29, "%s", 
	"github.com/drewjohnson2/mtop", 
	MT_PAIR_PRC_UNSEL_TEXT);
}
