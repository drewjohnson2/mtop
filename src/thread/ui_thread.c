#include <bits/time.h>
#include <ncurses.h>
#include <pthread.h>
#include <stddef.h>
#include <unistd.h>
#include <arena.h>
#include <assert.h>

#include "../../include/thread.h"
#include "../../include/window.h"
#include "../../include/mt_colors.h"
#include "../../include/startup.h"
#include "../../include/task.h"

void run_ui(
    UIData *ui
)
{
    import_colors();

    if (!mtopSettings->transparencyEnabled)
    {
		set_bg_colors(
		    ui->windows[CONTAINER_WIN]->window,
		    ui->windows[CPU_WIN]->window,
		    ui->windows[MEMORY_WIN]->window,
		    ui->windows[PRC_WIN]->window,
		    ui->windows[OPT_WIN]->window,
		    ui->windows[MENU_WIN]->window
		);
    }

    while (!SHUTDOWN_FLAG)
    {
		TaskGroup *tg = broker_read();

		UITask *task = tg->head;
		
		while (task)
		{
		    task->action(ui, task->data);
		    task = task->next;
		}

		tg->cleanup(&tg->a);
    	
    	usleep(DISPLAY_SLEEP_TIME);
    }
}
