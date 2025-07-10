#include <bits/time.h>
#include <ncurses.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <arena.h>
#include <assert.h>

#include "../../include/thread.h"
#include "../../include/window.h"
#include "../../include/monitor.h"
#include "../../include/thread_safe_queue.h"
#include "../../include/mt_colors.h"
#include "../../include/startup.h"
#include "../../include/task.h"


static void _setup_list_state(
    ProcessListState *listState,
    ProcessStats *curPrcs,
    const WindowData *prcWin
);

void run_ui(
    DisplayItems *di,
    ThreadSafeQueue *cpuQueue,
    ThreadSafeQueue *prcQueue,
    volatile ProcessInfoSharedData *prcInfoSd
)
{
    u64 memTotal = 0;
    const WindowData *cpuWin = di->windows[CPU_WIN];
    const WindowData *memWin = di->windows[MEMORY_WIN];
    const WindowData *prcWin = di->windows[PRC_WIN];
    const WindowData *optWin = di->windows[OPT_WIN];
    const WindowData *container = di->windows[CONTAINER_WIN];
    const u8 prcActive = mtopSettings->activeWindows[PRC_WIN];
    
    ProcessStats *prevPrcs = NULL;
    ProcessStats *curPrcs = NULL;
    
    // Arena stateArena = a_new(STATE_A_SZ);
    // 
    // // probably need to add some sort of shut down error
    // // handling here.
    // prevPrcs = peek(prcQueue, &procDataLock, &procQueueCondition);
    // dequeue(prcQueue, &procDataLock, &procQueueCondition);
    
    //curPrcs = prevPrcs;
    
    // ProcessListState *listState = a_alloc(
    // 	&stateArena,
    // 	sizeof(ProcessListState),
    // 	__alignof(ProcessListState)
    // );
    // 
    // _setup_list_state(listState, curPrcs, prcWin);
    import_colors();

    if (!mtopSettings->transparencyEnabled)
    {
	set_bg_colors(
	    container->window,
	    cpuWin->window,
	    memWin->window,
	    prcWin->window,
	    optWin->window
	);
    }

    print_header(container);
    print_footer(container);
    
    while (!SHUTDOWN_FLAG)
    {
	// if (RESIZE)
	// {
	//     resize_win(di); 
	//     _setup_list_state(listState, curPrcs, prcWin);
	// }

	print_uptime_ldAvg(container);
	print_time(container);

	TaskGroup *tg = peek(cpuQueue, &cpuQueueLock, &cpuQueueCondition);
	dequeue(cpuQueue, &cpuQueueLock, &cpuQueueCondition);

	// instead of passing in specific arena we could
	// pass in the arenas structure? Idk if I like that,
	// honestly.
	UITask *task = tg->tasks;
	
	while (task)
	{
	    task->action(di, task->data);
	    task = task->next;
	}

	tg->tasksComplete = 1;
	tg->cleanup();
    
	// Normally I'd remove the else case and put the
	// REFRESH_WIN(container->window) above the if statement.
	// For whatever reason that setup causes bad flickering
	// on some machines. I guess on faster machines display_options
	// takes enough time between the calls to REFRESH_WIN for ncurses
	// to repaint the screen.
	if (di->optionsVisible)
	{
	    display_options(di);	    
	    REFRESH_WIN(container->window);
	    REFRESH_WIN(optWin->window);
	}
	else 
	{
	    REFRESH_WIN(container->window);
	}

	// if (listState->infoVisible)
	// {
	//     prcInfoSd->needsFetch = 1;
	// }

	tg->tasksComplete = 1;
    
    	usleep(DISPLAY_SLEEP_TIME);
    }
}
