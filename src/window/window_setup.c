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

#define MIN_UTIL_WIN_WIDTH 55
#define MIN_UTIL_WIN_HEIGHT 17

#define FULL_WIDTH(container) ((container)->wWidth - 2)
#define HALF_WIDTH(container) ((FULL_WIDTH(container)) / 2)

#define FULL_HEIGHT(container) ((container)->wHeight - 2)
#define HALF_HEIGHT(container) ((FULL_HEIGHT(container)) / 2)

#define POS_X_START 1
#define POS_X_END(container, win) ((container)->wWidth - (win)->wWidth - 1)

#define POS_Y_START 1
#define POS_Y_BOTTOM(container, win) ((container)->wHeight - (win)->wHeight - 1)

typedef void (*LayoutHandler)(UIData *ui);

static void _setup_quarters_left(UIData *ui);
static void _setup_quarters_right(UIData *ui);
static void _setup_quarters_top(UIData *ui);
static void _setup_quarters_bottom(UIData *ui);
static void _setup_duo(UIData *ui);
static void _setup_single(UIData *ui);

LayoutHandler layout_fn_table[] = {
	_setup_quarters_left,
    _setup_quarters_right,
    _setup_quarters_top,
    _setup_quarters_bottom,
    _setup_duo,
    _setup_single
};

UIData * init_display_items(Arena *arena) 
{
    UIData *ui = a_alloc(arena, sizeof(UIData), __alignof(UIData));
    
    assert(ui);
    
    ui->optionsVisible = false;
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
    do {									\
		ui->windows[enumName] = a_alloc( 	\
    	    arena, 							\
    	    sizeof(WindowData), 			\
    	    __alignof(WindowData) 			\
    	); 									\
    	    								\
    	assert(ui->windows[enumName]);		\
    } while (0);
#include "../../include/tables/window_def_table.h"
#undef DEFINE_WINDOWS

    // remove magic number when I have a solid number nailed down.
    ui->menu = a_alloc(arena, sizeof(MenuData), __alignof(MenuData));
    ui->menu->items = a_alloc(arena, sizeof(MenuItem *) * 10, __alignof(MenuItem)); 

    for (size_t i = 0; i < 10; i++) 
		ui->menu->items[i] = a_alloc(arena, sizeof(MenuItem), __alignof(MenuItem));

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
    const Layout layout = mtopSettings->layout;
    const u8 winCount = mtopSettings->activeWindowCount;
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *optWin = ui->windows[OPT_WIN];
    WindowData *statTypeWin = ui->windows[STAT_TYPE_WIN];

    ui->windows[ui->windowOrder[0]]->active = true;
    if (winCount >= 2) ui->windows[ui->windowOrder[1]]->active = true;
    if (winCount == 3) ui->windows[ui->windowOrder[2]]->active = true;

    container->windowX = 0;
    container->windowY = 0;

    layout_fn_table[layout](ui);

    s16 optWinHeight = FLOAT_WIN_DEFAULT_H(container); 
    s16 optWinWidth = FLOAT_WIN_DEFAULT_W(container);

    optWinHeight = optWinHeight < MIN_UTIL_WIN_HEIGHT ? MIN_UTIL_WIN_HEIGHT : optWinHeight;
    optWinWidth = optWinWidth < MIN_UTIL_WIN_WIDTH ? MIN_UTIL_WIN_WIDTH : optWinWidth;

    size_floating_win(container, optWin, optWinHeight, optWinWidth);
    size_floating_win(container, statTypeWin, STAT_WIN_COUNT + 2, FLOAT_WIN_DEFAULT_W(container));
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

#define DEFINE_WINDOWS(winName, enumName) 				\
    WindowData *winName##Win = ui->windows[enumName]; 	\
														\
    winName##Win->window = subwin(						\
    	container->window,								\
    	winName##Win->wHeight,							\
    	winName##Win->wWidth,							\
    	winName##Win->windowY,							\
    	winName##Win->windowX							\
    );													\
														\
    assert(winName##Win->window);
#include "../../include/tables/window_def_table.h"
#undef DEFINE_WINDOWS
}

void size_floating_win(
    WindowData *container, 
    WindowData *win,
    s16 height,
    s16 width
)
{
    win->wWidth = width;
    win->wHeight = height;
    win->windowX = (container->wWidth / 2) - (win->wWidth / 2);
    win->windowY = (container->wHeight / 2) - (win->wHeight / 2);
}

static void _setup_quarters_left(UIData *ui)
{
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *winOne = ui->windows[ui->windowOrder[0]];
    WindowData *winTwo = ui->windows[ui->windowOrder[1]];
    WindowData *winThree = ui->windows[ui->windowOrder[2]];
    const u8 widthEven = container->wWidth % 2 == 0;
    const u8 heightEven = container->wHeight % 2 == 0;

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

static void _setup_quarters_right(UIData *ui)
{
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *winOne = ui->windows[ui->windowOrder[0]];
    WindowData *winTwo = ui->windows[ui->windowOrder[1]];
    WindowData *winThree = ui->windows[ui->windowOrder[2]];
    const u8 widthEven = container->wWidth % 2 == 0;
    const u8 heightEven = container->wHeight % 2 == 0;

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

static void _setup_quarters_top(UIData *ui)
{
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *winOne = ui->windows[ui->windowOrder[0]];
    WindowData *winTwo = ui->windows[ui->windowOrder[1]];
    WindowData *winThree = ui->windows[ui->windowOrder[2]];
    const u8 widthEven = container->wWidth % 2 == 0;
    const u8 heightEven = container->wHeight % 2 == 0;

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

static void _setup_quarters_bottom(UIData *ui)
{
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *winOne = ui->windows[ui->windowOrder[0]];
    WindowData *winTwo = ui->windows[ui->windowOrder[1]];
    WindowData *winThree = ui->windows[ui->windowOrder[2]];
    const u8 widthEven = container->wWidth % 2 == 0;
    const u8 heightEven = container->wHeight % 2 == 0;

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

static void _setup_duo(UIData *ui)
{
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *winOne = ui->windows[ui->windowOrder[0]];
    WindowData *winTwo = ui->windows[ui->windowOrder[1]];
    const u8 widthEven = container->wWidth % 2 == 0;
    const u8 heightEven = container->wHeight % 2 == 0;
    LayoutOrientation orientation = mtopSettings->orientation;
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

static void _setup_single(UIData *ui)
{
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *window = ui->windows[ui->windowOrder[0]];

    window->wWidth = FULL_WIDTH(container);
    window->wHeight = FULL_HEIGHT(container);
    window->windowX = POS_X_START;
    window->windowY = POS_Y_START;
    window->windowTitle = text(ui->windowOrder[0] + 19);
}
