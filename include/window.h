#ifndef WINDOW_H
#define WINDOW_H

#include <ncurses.h>
#include <arena.h>
#include <time.h>

#include "mt_type_defs.h"
#include "monitor.h"
#include "sorting.h"
#include "mt_colors.h"

#define INPUT_TIMEOUT_MS 575
#define STAT_WIN_COUNT 3

#define REFRESH_WIN(win) 	\
    do { 			\
	touchwin(win); 		\
	wrefresh(win); 		\
    } while (0) 		\

#define SET_COLOR(win, pair) wattron(win, COLOR_PAIR(pair))
#define UNSET_COLOR(win, pair) wattroff(win, COLOR_PAIR(pair))

#define PRINTFC(win, y, x, fmt, str, pair) 	\
    do { 					\
	SET_COLOR(win, pair); 			\
	mvwprintw(win, y, x, fmt, str); 	\
	UNSET_COLOR(win, pair); 		\
    } while (0)

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

typedef struct
{
    const char *displayString;
    mt_Window windowType;
    u8 isSelected;
} AddWindowMenuItem;

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
    u8 optionsVisible;
    u8 statTypesVisible;
    WindowData **windows;
    mt_Window windowOrder[3]; 
    WindowMode mode;
    mt_Window selectedWindow;
    AddWindowMenuItem **items;
} UIData;

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

typedef u16 (*compareFn)(WindowData *cmp, WindowData *cur);

extern u8 RESIZE;

//
//		window_setup.c
//
//
UIData * init_display_items(Arena *arena);
void init_windows(UIData *ui);
void init_window_dimens(UIData *ui);
void init_ncurses(WindowData *wd, SCREEN *screen);

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
void init_stat_menu_items(AddWindowMenuItem **items);
void print_header(const WindowData *wd);
void print_time(const WindowData *wd);
void print_uptime_ldAvg(const WindowData *wd);
void print_footer(const WindowData *wd);
void display_options(UIData *ui);
void display_stat_types(UIData *ui);
void set_bg_colors(
    WINDOW *container,
    WINDOW *cpuWin,
    WINDOW *memWin,
    WINDOW *prcWin,
    WINDOW *optWin,
    WINDOW *statTypeWin
);
void resize_win(UIData *ui);
void remove_win(UIData *ui, mt_Window winToRemove);
void add_win(UIData *ui, mt_Window winToAdd);
void init_menu_idx(AddWindowMenuItem **items);
void reset_menu_idx(AddWindowMenuItem **items);
void toggle_add_win_opts(AddWindowMenuItem **items);
mt_Window get_selected_window(UIData *ui, compareFn cmp);
mt_Window get_add_menu_selection(AddWindowMenuItem **items);

#endif
