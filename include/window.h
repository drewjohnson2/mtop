#ifndef WINDOW_H
#define WINDOW_H

#include <ncurses.h>
#include <arena.h>
#include <time.h>

#include "mt_type_defs.h"
#include "monitor.h"
#include "sorting.h"
#include "mt_colors.h"
#include "startup.h"

#define INPUT_TIMEOUT_MS 575
#define STAT_WIN_COUNT 3

#define REFRESH_WIN(win) 	\
    do { 					\
		touchwin(win); 		\
		wrefresh(win); 		\
    } while (0) 			\

#define SET_COLOR(win, pair) wattron(win, COLOR_PAIR(pair))
#define UNSET_COLOR(win, pair) wattroff(win, COLOR_PAIR(pair))

#define PRINTFC(win, y, x, fmt, str, pair) 	\
    do { 									\
		SET_COLOR(win, pair); 				\
		mvwprintw(win, y, x, fmt, str); 	\
		UNSET_COLOR(win, pair); 			\
    } while (0)

#define FLOAT_WIN_DEFAULT_W(container) container->wWidth / 4
#define FLOAT_WIN_DEFAULT_H(container) container->wHeight / 3

struct UIData;
typedef struct UIData UIData;

typedef enum _mt_window 
{
#define DEFINE_WINDOWS(winName, enumName) enumName,
    CONTAINER_WIN,
#include "../include/tables/window_def_table.h"
    WINDOW_ID_MAX
#undef DEFINE_WINDOWS
} mt_Window;

typedef enum 
{
    NORMAL,
    ARRANGE
} WindowMode;

typedef union
{
    mt_Window windowType;
    Layout layout;
    LayoutOrientation orientation;
} MenuItemValue;

typedef struct
{
    const char *displayString;
    MenuItemValue returnValue;
    u8 isSelected;
    u8 isHidden;
} MenuItem;

typedef struct
{
    WINDOW *window;
    u16 wHeight, wWidth;
    u16 windowX, windowY;
    u8 active;
    const char *windowTitle;
} WindowData;

typedef struct
{
    u8 isVisible;
    u8 menuItemCount;
    MenuItem **items;
    void (*on_select)(UIData *, MenuItemValue);
} MenuData;

struct UIData
{
    u8 optionsVisible;
    u8 reinitListState;
    mt_Window selectedWindow;
    mt_Window windowOrder[3]; 
    MenuData *menu; 
    WindowData **windows;
    WindowMode mode;
};

typedef struct _graph_point
{
    float percent;
    struct _graph_point *next;
} GraphPoint;

typedef struct
{
    size_t graphPointCount;
    GraphPoint *head;
} GraphData;

typedef struct
{
    float cpuPercentage;
    float memPercentage;
    char *command;
    char state;
    s32 ppid;
    s32 threads;
    u32 pid;
    u64 utime;
    u64 stime;
    u64 vmRss;
    u64 vmSize;
    u64 vmLock;
    u64 vmData;
    u64 vmStack;
    u64 vmSwap;
    u64 vmExe;
    u64 vmLib;

} ProcessStatsViewData;

typedef struct
{
    s8 selectedIndex;
    s8 pageStartIdx;
    s8 pageEndIdx;
    s8 count;
    u8 totalPages;
    u8 activePage;
    u8 pageSize;
    s8 timeoutActive;
    u8 infoVisible;
    u32 selectedPid;
    char cmdBuffer;
    SortOrder sortOrder;
    struct timespec timeoutStart;

    int (*sortFn)(const void *a, const void *b);
} ProcessListState;

typedef u8 (*WinPosComparisonFn)(WindowData *cmp, WindowData *cur);

extern u8 RESIZE;
extern u8 REINIT;

//
//		window_setup.c
//
//
UIData * init_display_items(Arena *arena);
void init_windows(UIData *ui);
void init_window_dimens(UIData *ui);
void init_ncurses(WindowData *wd, SCREEN *screen);
void size_floating_win(
    WindowData *container, 
    WindowData *win,
    s16 height,
    s16 width
);

//
//		graph.c
//
//
s8 graph_render(
    Arena *arena,
    GraphData *gd,
    const WindowData *wd,
    MT_Color_Pairs gpColor,
    MT_Color_Pairs headerColor,
    u8 windowSelected
);
s8 add_graph_point(Arena *arena, GraphData *gd, float percentage, u8 winActive);

//
//		prc_list.c
//
//
void print_stats(
    ProcessListState *state,
    const WindowData *wd,
    ProcessStatsViewData **vd,
    s16 count,
    u8 winSelected
);
void set_prc_view_data(
    Arena *scratch,
    ProcessStatsViewData **vd,
    ProcessesSummary *curPrcs,
    ProcessesSummary *prevPrcs,
    u64 memTotal
);
void adjust_state(ProcessListState *state, ProcessesSummary *stats);
void set_start_end_idx(ProcessListState *state);
void show_prc_info(ProcessStatsViewData *vd, const WindowData *wd, u8 winSelected);

// why are input functions being declared in the window header?
//
//		input.c
//
//
void read_normal_input(
    WINDOW *win,
    ProcessListState *state,
    UIData *ui
);
void read_arrange_input(UIData *ui);

//
//		window_util.c
//
//
void display_normal_options(UIData *ui);
void display_arrange_options(UIData *ui);
void set_bg_colors(
    WINDOW *container,
    WINDOW *cpuWin,
    WINDOW *memWin,
    WINDOW *prcWin,
    WINDOW *optWin,
    WINDOW *statTypeWin
);
void resize_windows(UIData *ui);
void remove_win(UIData *ui, mt_Window winToRemove);
void add_win(UIData *ui, mt_Window winToAdd);
void swap_windows(UIData *ui, mt_Window windowToSwap);
void reinit_window(UIData *ui);
mt_Window get_selected_window(UIData *ui, WinPosComparisonFn cmp);

#endif
