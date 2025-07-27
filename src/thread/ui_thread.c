#include <bits/time.h>
#include <ncurses.h>
#include <pthread.h>
#include <stddef.h>
#include <unistd.h>
#include <arena.h>
#include <assert.h>

#include "../../include/thread.h"
#include "../../include/window.h"
#include "../../include/thread_safe_queue.h"
#include "../../include/mt_colors.h"
#include "../../include/startup.h"
#include "../../include/task.h"

void run_ui(
    DisplayItems *di,
    ThreadSafeQueue *taskQueue
)
{
    const WindowData *cpuWin = di->windows[CPU_WIN];
    const WindowData *memWin = di->windows[MEMORY_WIN];
    const WindowData *prcWin = di->windows[PRC_WIN];
    const WindowData *optWin = di->windows[OPT_WIN];
    const WindowData *container = di->windows[CONTAINER_WIN];
    
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
	print_uptime_ldAvg(container);
	print_time(container);

	TaskGroup *tg = peek(taskQueue, &taskQueueLock, &taskQueueCondition);
	dequeue(taskQueue, &taskQueueLock, &taskQueueCondition);

	// instead of passing in specific arena we could
	// pass in the arenas structure? Idk if I like that,
	// honestly.
	UITask *task = tg->head;
	
	while (task)
	{
	    task->action(di, task->data);
	    task = task->next;
	}

	tg->cleanup(&tg->a);

	tg->tasksComplete = 1;
    
    	usleep(DISPLAY_SLEEP_TIME);
    }
}
