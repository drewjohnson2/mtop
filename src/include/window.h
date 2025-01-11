#ifndef WINDOW_H
#define WINDOW_H

#include <ncurses.h>
#include <arena.h>
#include <time.h>

#include "mt_type_defs.h"
#include "monitor.h"

#define INPUT_TIMEOUT_MS 575

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


typedef struct _window_data
{
    WINDOW *window;
    u16 wHeight, wWidth;
    u16 windowX, windowY;
    u16 paddingTop;
    u16 paddingBottom;
    u16 paddingRight;
    u16 paddingLeft;
    char *windowTitle;
} WindowData;

typedef struct _display_items
{
    size_t windowCount;
    u8 optionsVisible;
    WindowData **windows;
} DisplayItems;

typedef struct _graph_point
{
    float percent;
    struct _graph_point *next;
} GraphPoint;

typedef struct _graph_data 
{
    size_t graphPointCount;
    GraphPoint *head;
} GraphData;

typedef struct _stats_view_data 
{
    char *command;
    u32 pid;
    float cpuPercentage;
    float memPercentage;
} ProcessStatsViewData;

typedef struct _process_list_state
{
    s8 selectedIndex;
    s8 firstIndexDisplayed;
    s8 lastIndexDisplayed;
    s8 maxIndex;
    s8 numOptsVisible;
    s8 timeoutActive;
    char cmdBuffer;
    struct timespec timeoutStart;

    int (*sortFunc)(const void *a, const void *b);
} ProcessListState;

//
//		window_setup.c
//
//
DisplayItems * init_display_items(Arena *arena);
void init_windows(DisplayItems *di);
void init_window_dimens(DisplayItems *di);
void init_ncurses(WindowData *wd, SCREEN *screen);
void print_header(WindowData *wd);
void print_time(WindowData *wd);
void print_footer(WindowData *wd);
void display_options(DisplayItems *di);

//
//		graph.c
//
//
s8 graph_render(Arena *arena, GraphData *gd, WindowData *wd);
s8 add_graph_point(Arena *arena, GraphData *gd, float percentage);

//
//		prc_list.c
//
//
void print_stats(
    ProcessListState *state,
    WindowData *wd,
    ProcessStatsViewData **vd,
    s16 count
);
void set_prc_view_data(
    Arena *scratch,
    ProcessStatsViewData **vd,
    ProcessStats *curPrcs,
    ProcessStats *prevPrcs,
    u64 memTotal
);
void read_input(
    WINDOW *win,
    ProcessListState *state,
    DisplayItems *di,
    ProcessStatsViewData **vd
);
void adjust_state(ProcessListState *state, ProcessStats *stats);

#endif
