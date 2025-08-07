#include <locale.h>
#include <ncurses.h>
#include <arena.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>

#include "../../include/window.h"
#include "../../include/startup.h"
#include "../../include/text.h"

#define FULL_WIDTH(container) ((container)->wWidth - 2)
#define HALF_WIDTH(container) ((FULL_WIDTH(container)) / 2)

#define FULL_HEIGHT(container) ((container)->wHeight - 2)
#define HALF_HEIGHT(container) ((FULL_HEIGHT(container)) / 2)

#define POS_X_START 1
#define POS_X_END(container, win) ((container)->wWidth - (win)->wWidth - 1)

#define POS_Y_START 1
#define POS_Y_BOTTOM(container, win) ((container)->wHeight - (win)->wHeight - 1)

#define UTIL_W_DEFAULT(container) container->wWidth / 4
#define UTIL_H_DEFAULT(container) container->wHeight / 4

static void _setup_util_win(
    WindowData *container, 
    WindowData *win,
    u8 height,
    u8 width,
    u8 enforceMinH,
    u8 enforceMinW
);
static void _setup_quarters_left(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    UIData *ui,
    const u8 widthEven,
    const u8 heightEven
);
static void _setup_quarters_right(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    UIData *ui,
    const u8 widthEven,
    const u8 heightEven
);
static void _setup_quarters_top(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    UIData *ui,
    const u8 widthEven,
    const u8 heightEven
);
static void _setup_quarters_bottom(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    UIData *ui,
    const u8 widthEven,
    const u8 heightEven
);
static void _setup_duo(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    UIData *ui,
    const u8 widthEven,
    const u8 heightEven,
    const LayoutOrientation orientation
);

UIData * init_display_items(Arena *arena) 
{
    UIData *ui = a_alloc(arena, sizeof(UIData), __alignof(UIData));
    
    assert(ui);
    
    ui->optionsVisible = 0;
    ui->windowOrder[0] = WINDOW_ID_MAX;
    ui->windowOrder[1] = WINDOW_ID_MAX;
    ui->windowOrder[2] = WINDOW_ID_MAX;
    
    ui->windows = a_alloc(
    	arena,
    	sizeof(WindowData *) * WINDOW_ID_MAX,
    	__alignof(WindowData *)
    );
    
    assert(ui->windows);
    
    ui->windows[CONTAINER_WIN] = a_alloc(
    	arena, 
    	sizeof(WindowData),
    	__alignof(WindowData)
    );

#define DEFINE_WINDOWS(winName, enumName) 	\
    do {					\
	ui->windows[enumName] = a_alloc( 	\
    	    arena, 				\
    	    sizeof(WindowData), 		\
    	    __alignof(WindowData) 		\
    	); 					\
    	    					\
    	assert(ui->windows[enumName]);		\
    } while (0);
#include "../../include/tables/window_def_table.h"
#undef DEFINE_WINDOWS

    ui->items = a_alloc(arena, sizeof(AddWindowMenuItem *) * STAT_WIN_COUNT,
	__alignof(AddWindowMenuItem)); 

    for (size_t i = 0; i < STAT_WIN_COUNT; i++) 
	 ui->items[i] = a_alloc(arena, sizeof(AddWindowMenuItem), __alignof(AddWindowMenuItem));

    return ui;
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
void init_window_dimens(UIData *ui)
{
    const LayoutOrientation orientation = mtopSettings->orientation;    
    const Layout layout = mtopSettings->layout;
    const u8 winCount = mtopSettings->activeWindowCount;
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *winOne = ui->windows[ui->windowOrder[0]];
    WindowData *winTwo = winCount >= 2 ? ui->windows[ui->windowOrder[1]] : NULL;
    WindowData *winThree = winCount == 3 ? ui->windows[ui->windowOrder[2]] : NULL;
    WindowData *optWin = ui->windows[OPT_WIN];
    WindowData *statTypeWin = ui->windows[STAT_TYPE_WIN];
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
		ui,
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
		ui,
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
		ui,
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
		ui,
		widthEven,
		heightEven
	    );

	    break;
	case DUO:
	    _setup_duo(
		container,
		winOne,
		winTwo,
		ui,
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
	    winOne->windowTitle = text(ui->windowOrder[0] + 19);

	    break;
	default:
	    exit(0);
    }

    _setup_util_win(container, optWin, UTIL_H_DEFAULT(container), UTIL_W_DEFAULT(container), 1, 1);
    _setup_util_win(container, statTypeWin, STAT_WIN_COUNT + 2, UTIL_W_DEFAULT(container), 0, 1);
}

void init_windows(UIData *ui) 
{
    WindowData *container = ui->windows[CONTAINER_WIN];
    
     container->window = newwin(
    	container->wHeight,
    	container->wWidth,
    	container->windowY,
    	container->windowX
    );
    
    assert(container->window);
    nodelay(container->window, TRUE);

#define DEFINE_WINDOWS(winName, enumName) 		\
    WindowData *winName##Win = ui->windows[enumName]; 	\
							\
    winName##Win->window = subwin(			\
    	container->window,				\
    	winName##Win->wHeight,				\
    	winName##Win->wWidth,				\
    	winName##Win->windowY,				\
    	winName##Win->windowX				\
    );							\
							\
    assert(winName##Win->window);
#include "../../include/tables/window_def_table.h"
#undef DEFINE_WINDOWS
}

// Need to just combine these two functions. It's the same damn thing.
static void _setup_util_win(
    WindowData *container, 
    WindowData *win,
    u8 height,
    u8 width,
    u8 enforceMinH,
    u8 enforceMinW
)
{
    win->wWidth = width;
    win->wHeight = height;

    if (win->wHeight < 16 && enforceMinH) win->wHeight = 17;
    if (win->wWidth < 50 && enforceMinW) win->wWidth = 55;

    win->windowX = (container->wWidth / 2) - (win->wWidth / 2);
    win->windowY = (container->wHeight / 2) - (win->wHeight / 2);
}

static void _setup_stat_type_win(WindowData *container, WindowData *statTypeWin)
{
    statTypeWin->wWidth = container->wWidth / 4;
    statTypeWin->wHeight = 5;

    if (statTypeWin->wWidth < 50) statTypeWin->wWidth = 55;

    statTypeWin->windowX = (container->wWidth / 2) - (statTypeWin->wWidth / 2);
    statTypeWin->windowY = (container->wHeight / 2) - (statTypeWin->wHeight / 2);
}

static void _setup_quarters_left(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    UIData *ui,
    const u8 widthEven,
    const u8 heightEven
)
{
    winOne->wWidth = HALF_WIDTH(container);
    winOne->wHeight = HALF_HEIGHT(container);
    winOne->windowX = POS_X_START;
    winOne->windowY = POS_Y_START;
    winOne->windowTitle = text(ui->windowOrder[0] + 19);
    
    winTwo->wWidth = widthEven ?
        HALF_WIDTH(container) :
    	HALF_WIDTH(container) + 1;
    winTwo->wHeight = FULL_HEIGHT(container);
    winTwo->windowX = POS_X_END(container, winTwo);
    winTwo->windowY = POS_X_START;
    winTwo->windowTitle = text(ui->windowOrder[1] + 19);
    
    winThree->wWidth = HALF_WIDTH(container);
    winThree->wHeight = heightEven ? 
        HALF_HEIGHT(container) :
        HALF_HEIGHT(container) + 1;
    winThree->windowX = POS_X_START;
    winThree->windowY = POS_Y_BOTTOM(container, winThree);
    winThree->windowTitle = text(ui->windowOrder[2] + 19);
}

static void _setup_quarters_right(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    UIData *ui,
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
    winOne->windowTitle = text(ui->windowOrder[0] + 19);
    
    winTwo->wWidth = HALF_WIDTH(container); 
    winTwo->wHeight = HALF_HEIGHT(container);
    winTwo->windowX = POS_X_END(container, winTwo);
    winTwo->windowY = POS_Y_START; 
    winTwo->windowTitle = text(ui->windowOrder[1] + 19);
    
    winThree->wWidth = HALF_WIDTH(container);
    winThree->wHeight = heightEven ?
        HALF_HEIGHT(container) :
        HALF_HEIGHT(container) + 1;
    winThree->windowX = POS_X_END(container, winThree);
    winThree->windowY = POS_Y_BOTTOM(container, winThree);
    winThree->windowTitle = text(ui->windowOrder[2] + 19);
}

static void _setup_quarters_top(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    UIData *ui,
    const u8 widthEven,
    const u8 heightEven
)
{
    winOne->wWidth = HALF_WIDTH(container);
    winOne->wHeight = HALF_HEIGHT(container);
    winOne->windowX = POS_X_START;
    winOne->windowY = POS_Y_START;
    winOne->windowTitle = text(ui->windowOrder[0] + 19);
    
    winTwo->wWidth = widthEven ?
        HALF_WIDTH(container) :
        HALF_WIDTH(container) + 1;
    winTwo->wHeight = HALF_HEIGHT(container);
    winTwo->windowX = POS_X_END(container, winTwo);
    winTwo->windowY = POS_Y_START;
    winTwo->windowTitle = text(ui->windowOrder[1] + 19);
    
    winThree->wWidth = FULL_WIDTH(container);
    winThree->wHeight = heightEven ?
        HALF_HEIGHT(container) :
        HALF_HEIGHT(container) + 1;
    winThree->windowX = POS_X_START;
    winThree->windowY = POS_Y_BOTTOM(container, winThree);
    winThree->windowTitle = text(ui->windowOrder[2] + 19);
}

static void _setup_quarters_bottom(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    WindowData *winThree,
    UIData *ui,
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
    winOne->windowTitle = text(ui->windowOrder[0] + 19);
    
    winTwo->wWidth = HALF_WIDTH(container);
    winTwo->wHeight = HALF_HEIGHT(container);
    winTwo->windowX = POS_X_START;
    winTwo->windowY = POS_Y_BOTTOM(container, winTwo); 
    winTwo->windowTitle = text(ui->windowOrder[1] + 19);
    
    winThree->wWidth = widthEven ?
        HALF_WIDTH(container) :
        HALF_WIDTH(container) + 1;
    winThree->wHeight = HALF_HEIGHT(container); 
    winThree->windowX = POS_X_END(container, winThree);
    winThree->windowY = POS_Y_BOTTOM(container, winThree);
    winThree->windowTitle = text(ui->windowOrder[2] + 19);
}

static void _setup_duo(
    WindowData *container,
    WindowData *winOne,
    WindowData *winTwo,
    UIData *ui,
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
    winOne->windowTitle = text(ui->windowOrder[0] + 19);
                          
    winTwo->wWidth = widthTwo;
    winTwo->wHeight = heightTwo;
    winTwo->windowX = isHzl ? POS_X_START : POS_X_END(container, winTwo);
    winTwo->windowY = isHzl ? POS_Y_BOTTOM(container, winTwo) : POS_Y_START;
    winTwo->windowTitle = text(ui->windowOrder[1] + 19);
}
