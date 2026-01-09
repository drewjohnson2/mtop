#include <locale.h>
#include <ncurses.h>
#include <arena.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef __linux__
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#endif

#include "../../include/window.h"
#include "../../include/startup.h"
#include "../../include/text.h"

#define FULL_WIDTH(container) ((container)->wWidth - 2)
#define HALF_WIDTH(container) ((FULL_WIDTH(container)) / 2)
#define FULL_HEIGHT(container) ((container)->wHeight - 2)
#define HALF_HEIGHT(container) ((FULL_HEIGHT(container)) / 2)
#define POS_X_END(container, win) ((container)->wWidth - (win)->wWidth - 1)
#define POS_Y_BOTTOM(container, win) ((container)->wHeight - (win)->wHeight - 1)
#define POS_AXIS_START 1
#define WINDOW_ID_TEXT_OFFSET 19
#define WIN_ONE_IDX 0
#define ORIENTATION_CNT 2
#define LAYOUT_CNT 6
#define MAX_WIN_CNT 3

typedef enum
{
	FULL_HEIGHT,
	HALF_HEIGHT,
	HALF_HEIGHT_PLUS_ONE,
	FULL_WIDTH,
	HALF_WIDTH,
	HALF_WIDTH_PLUS_ONE,
	AXIS_START,
	AXIS_X_END,
	AXIS_Y_END
} WindowGeometry;

typedef struct
{
	WindowGeometry height;
	WindowGeometry width;
	WindowGeometry posX;
	WindowGeometry posY;
} PlacementDetails;

static PlacementDetails sizeTable[ORIENTATION_CNT][LAYOUT_CNT][MAX_WIN_CNT] = 
{
	{	// Horizontal
		{ /* N/A */ },{ /* N/A */ },
		{ // Quarters Top
			{ HALF_HEIGHT, HALF_WIDTH, AXIS_START, AXIS_START },
			{ HALF_HEIGHT, HALF_WIDTH_PLUS_ONE, AXIS_X_END, AXIS_START },
			{ HALF_HEIGHT_PLUS_ONE, FULL_WIDTH, AXIS_START, AXIS_Y_END }
		},
		{ // Quarters Bottom 
			{ HALF_HEIGHT_PLUS_ONE, FULL_WIDTH, AXIS_START, AXIS_START },
			{ HALF_HEIGHT, HALF_WIDTH, AXIS_START, AXIS_Y_END },
			{ HALF_HEIGHT, HALF_WIDTH_PLUS_ONE, AXIS_X_END, AXIS_Y_END }
		},
		{ // Duo
			{ HALF_HEIGHT_PLUS_ONE, FULL_WIDTH, AXIS_START, AXIS_START },
			{ HALF_HEIGHT, FULL_WIDTH, AXIS_START, AXIS_Y_END }
		},
		{ // Single
			{ FULL_HEIGHT, FULL_WIDTH, AXIS_START, AXIS_START } 
		}
	},
	{	// Vertical
		{ // Quarters Left
			{ HALF_HEIGHT, HALF_WIDTH, AXIS_START, AXIS_START },
			{ FULL_HEIGHT, HALF_WIDTH_PLUS_ONE, AXIS_X_END, AXIS_START },
			{ HALF_HEIGHT_PLUS_ONE, HALF_WIDTH, AXIS_START, AXIS_Y_END }
		},
		{ // Quarters Right
			{ FULL_HEIGHT, HALF_WIDTH_PLUS_ONE, AXIS_START, AXIS_START },
			{ HALF_HEIGHT, HALF_WIDTH, AXIS_X_END, AXIS_START },
			{ HALF_HEIGHT_PLUS_ONE, HALF_WIDTH, AXIS_X_END, AXIS_Y_END }
		},
		{ /* N/A */ }, { /* N/A */ },
		{ // Duo
			{ FULL_HEIGHT, HALF_WIDTH_PLUS_ONE, AXIS_START, AXIS_START },
			{ FULL_HEIGHT, HALF_WIDTH, AXIS_X_END, AXIS_START}
		},
		{ // Single
			{ FULL_HEIGHT, FULL_WIDTH, AXIS_START, AXIS_START } 
		}

	}
};

static u16 _get_size(WindowData *container, WindowGeometry size);
static u16 _get_position(WindowData *container, WindowData *win, WindowGeometry size);
static void _size_window(UIData *ui, PlacementDetails winDetails[MAX_WIN_CNT]);

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

void init_window_dimens(UIData *ui)
{
    const Layout layout = mtopSettings->layout;
    const u8 winCount = mtopSettings->activeWindowCount;
    const u8 verticalPadding = 2;
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *optWin = ui->windows[OPT_WIN];
    WindowData *menuWin = ui->windows[MENU_WIN];

    ui->windows[ui->windowOrder[0]]->active = true;
    if (winCount >= 2) ui->windows[ui->windowOrder[1]]->active = true;
    if (winCount == 3) ui->windows[ui->windowOrder[2]]->active = true;

    container->windowX = 0;
    container->windowY = 0;

	_size_window(ui, sizeTable[mtopSettings->orientation][layout]);

    size_floating_win(container, optWin, MIN_NORMAL_MODE_UTIL_WIN_HEIGHT, MIN_UTIL_WIN_WIDTH);
    size_floating_win(container, menuWin, STAT_WIN_COUNT + verticalPadding, FLOAT_WIN_DEFAULT_W(container));
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

	wnoutrefresh(win->window);
}

static void _size_window(UIData *ui, PlacementDetails winDetails[3])
{
	WindowData *container = ui->windows[CONTAINER_WIN];

	for (size_t i = WIN_ONE_IDX; i < STAT_WIN_COUNT; i++)
	{
		if (ui->windowOrder[i] == WINDOW_ID_MAX) break;

		mt_Window windowId= ui->windowOrder[i];
		WindowData *win = ui->windows[windowId];

		win->wHeight = _get_size(container, winDetails[i].height);
		win->wWidth = _get_size(container, winDetails[i].width);
		win->windowX = _get_position(container, win, winDetails[i].posX);
		win->windowY = _get_position(container, win, winDetails[i].posY);
		win->windowTitle = text(ui->windowOrder[i] + WINDOW_ID_TEXT_OFFSET);
	}
}

static u16 _get_size(WindowData *container, WindowGeometry size)
{
	const u8 widthEven = container->wWidth % 2 == 0;
    const u8 heightEven = container->wHeight % 2 == 0;

	switch (size)
	{
		case FULL_HEIGHT: return FULL_HEIGHT(container);
		case FULL_WIDTH:  return FULL_WIDTH(container);
		case HALF_HEIGHT: return HALF_HEIGHT(container);
		case HALF_WIDTH:  return HALF_WIDTH(container);
		case HALF_WIDTH_PLUS_ONE: return HALF_WIDTH(container) + !widthEven;
		case HALF_HEIGHT_PLUS_ONE: return HALF_HEIGHT(container) + !heightEven;
		default: return 0;
	}
}

static u16 _get_position(WindowData *container, WindowData *win, WindowGeometry size)
{
	switch (size)
	{
		case AXIS_START: return POS_AXIS_START;
		case AXIS_X_END: return POS_X_END(container, win);
		case AXIS_Y_END: return POS_Y_BOTTOM(container, win);
		default: return 0;
	}
}
