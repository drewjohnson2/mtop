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
    UIData *ui,
    ThreadSafeQueue *taskQueue
)
{
    const WindowData *cpuWin = ui->windows[CPU_WIN];
    const WindowData *memWin = ui->windows[MEMORY_WIN];
    const WindowData *prcWin = ui->windows[PRC_WIN];
    const WindowData *optWin = ui->windows[OPT_WIN];
    const WindowData *statTypeWin = ui->windows[STAT_TYPE_WIN];
    const WindowData *container = ui->windows[CONTAINER_WIN];
    
    import_colors();

    if (!mtopSettings->transparencyEnabled)
    {
		set_bg_colors(
		    container->window,
		    cpuWin->window,
		    memWin->window,
		    prcWin->window,
		    optWin->window,
		    statTypeWin->window
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
		    task->action(ui, task->data);
		    task = task->next;
		}

		tg->cleanup(&tg->a);

		tg->tasksComplete = 1;
    	
    		usleep(DISPLAY_SLEEP_TIME);
    }
}
