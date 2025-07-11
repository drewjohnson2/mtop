#define _GNU_SOURCE

#include <getopt.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>
#include <signal.h>

#include "../include/startup.h"
#include "../include/monitor.h"
#include "../include/thread_safe_queue.h"
#include "../include/window.h"
#include "../include/thread.h"

#define WINDOW_A_SZ 256
#define CPU_A_SZ sizeof(CpuStats)
#define MEM_A_SZ sizeof(MemoryStats)
#define GRAPH_A_SZ sizeof(GraphData)
#define QUEUE_A_SZ sizeof(ThreadSafeQueue)
#define GENERAL_A_SZ 256 * 20
#define PRC_A_SZ (MAX_PROCS * sizeof(ProcessList *)) + (MAX_PROCS * sizeof(ProcessList))
#define CPU_POINT_A_SZ sizeof(GraphPoint)
#define MEM_POINT_A_SZ sizeof(GraphPoint)
#define STATE_A_SZ sizeof(ProcessListState) + __alignof(ProcessListState)

typedef struct _ui_thread_args
{
    DisplayItems *di;
    ThreadSafeQueue *cpuQueue;
} UIThreadArgs;

typedef struct _io_thread_args
{
    mtopArenas *arenas;
    ThreadSafeQueue *cpuQueue;
    WindowData **windows;
} IOThreadArgs;

Arena windowArena;
Arena cpuArena;
Arena memArena; 
Arena cpuGraphArena;     
Arena memoryGraphArena;  
Arena prcArena;
Arena queueArena;
Arena cpuPointArena;
Arena memPointArena;
Arena general;
Arena stateArena;

ThreadSafeQueue *cpuQueue;

volatile Settings *mtopSettings;

static void * _ui_thread_run(void *arg);
static void * _io_thread_run(void *arg);
static void _set_active_window(mt_Window *windows, mt_Window winToAdd);
static u8 _get_option_after_flag_with_space(char **optarg, char **argv, u8 argc, u8 optind);

void _handle_resize(int sig);

void run(int argc, char **argv) 
{
    int option_index = 0;
    s8 arg;

    FILE *tty = fopen("/dev/tty", "r+");
    SCREEN *screen = newterm(NULL, tty, tty);

    // If program starts crashing randomly after
    // any period of time it's probably time to roll
    // back to a different commit on this allocation block.
    windowArena = a_new(WINDOW_A_SZ);
    cpuArena = a_new(CPU_A_SZ);
    memArena = a_new(MEM_A_SZ);
    cpuGraphArena = a_new(GRAPH_A_SZ);     
    memoryGraphArena = a_new(GRAPH_A_SZ);  
    prcArena = a_new(PRC_A_SZ);
    queueArena = a_new(QUEUE_A_SZ);
    cpuPointArena = a_new(CPU_POINT_A_SZ);
    memPointArena = a_new(MEM_POINT_A_SZ);
    stateArena = a_new(STATE_A_SZ);
    general = a_new(GENERAL_A_SZ);
    
    DisplayItems *di = init_display_items(&windowArena);
    mtopArenas *arenas = a_alloc(&general, sizeof(mtopArenas), __alignof(mtopArenas));

    arenas->general = &general;
    arenas->cpuArena = &cpuArena;
    arenas->windowArena = &windowArena;
    arenas->queueArena = &queueArena;
    arenas->memArena = &memArena;
    arenas->cpuGraphArena = &cpuGraphArena;
    arenas->memoryGraphArena  = &memoryGraphArena;
    arenas->prcArena = &prcArena;
    arenas->cpuPointArena = &cpuPointArena;
    arenas->memPointArena = &memPointArena;
    arenas->stateArena = &stateArena;

    cpuQueue = a_alloc(
    	&queueArena,
    	sizeof(ThreadSafeQueue),
    	__alignof(ThreadSafeQueue)
    );

    mtopSettings = a_alloc(&general, sizeof(Settings), __alignof(Settings));
    mtopSettings->orientation = HORIZONTAL;
    mtopSettings->layout = QUARTERS_BOTTOM;
    mtopSettings->activeWindowCount = 0;
    mtopSettings->activeWindows[CPU_WIN] = 0;
    mtopSettings->activeWindows[MEMORY_WIN] = 0;
    mtopSettings->activeWindows[PRC_WIN] = 0;

    static struct option long_options[] = 
    {
	{ "transparent", no_argument, NULL, 't' },
	{ "cpu", no_argument, NULL, 'c'},
	{ "memory", no_argument, NULL, 'm'},
	{ "process", no_argument, NULL, 'p'},
	{ "vertical", optional_argument, NULL, 'v'},
	{ "horizontal", optional_argument, NULL, 'h' },
	{ NULL, no_argument, NULL, 0 }
    };

    while((arg = getopt_long(argc, argv, "tcmpv::h::", long_options, &option_index)) != -1)
    {
	switch (arg) 
    	{
    	    case 't':
		mtopSettings->transparencyEnabled = 1;
    	        break;
	    case 'c':
		mtopSettings->activeWindows[CPU_WIN] = 1;
		_set_active_window(di->selectedWindows, CPU_WIN);
		break;
	    case 'm':
		mtopSettings->activeWindows[MEMORY_WIN] = 1;
		_set_active_window(di->selectedWindows, MEMORY_WIN);
		break;
	    case 'p':
		mtopSettings->activeWindows[PRC_WIN] = 1;
		_set_active_window(di->selectedWindows, PRC_WIN);
		break;
	    case 'h':
		mtopSettings->orientation = HORIZONTAL;
		mtopSettings->layout = QUARTERS_BOTTOM;

		if (!_get_option_after_flag_with_space(&optarg, argv, (u8)argc, optind)) break;

		if (strcmp(optarg, "top") == 0)
		{
		    mtopSettings->layout = QUARTERS_TOP;
		    break;
		}

		break;
	    case 'v':
		mtopSettings->orientation = VERTICAL;
		mtopSettings->layout = QUARTERS_LEFT;

		if (!_get_option_after_flag_with_space(&optarg, argv, (u8)argc, optind)) break;
		
		if (strcmp(optarg, "right") == 0)
		{
		    mtopSettings->layout = QUARTERS_RIGHT;
		    break;
		}

		break;
    	    default:
    	        break;
    	}
    }

    if (
	di->selectedWindows[0] == WINDOW_ID_MAX && 
	di->selectedWindows[1] == WINDOW_ID_MAX && 
	di->selectedWindows[2] == WINDOW_ID_MAX
    )
    {
	mtopSettings->activeWindowCount = 3;

	di->selectedWindows[0] = CPU_WIN;
	di->selectedWindows[1] = MEMORY_WIN;
	di->selectedWindows[2] = PRC_WIN;

	mtopSettings->activeWindows[CPU_WIN] = 1;
	mtopSettings->activeWindows[MEMORY_WIN] = 1;
	mtopSettings->activeWindows[PRC_WIN] = 1;
    }

    if (mtopSettings->activeWindowCount == 2) mtopSettings->layout = DUO;
    else if (mtopSettings->activeWindowCount == 1) mtopSettings->layout = SINGLE;

    signal(SIGWINCH, _handle_resize);
    init_ncurses(di->windows[CONTAINER_WIN], screen);
    init_window_dimens(di);
    init_windows(di);
    
    UIThreadArgs uiArgs = 
    {
    	.di = di,
    	.cpuQueue = cpuQueue,
    };
    
    IOThreadArgs ioArgs = 
    {
	.arenas = arenas,
    	.cpuQueue = cpuQueue,
	.windows = di->windows
    };
    
    pthread_t ioThread;
    pthread_t ui_thread;

    // setup
    mutex_init();
    pthread_create(&ioThread, NULL, _io_thread_run, (void *)&ioArgs);
    pthread_create(&ui_thread, NULL, _ui_thread_run, (void *)&uiArgs);

    // tear down
    pthread_join(ioThread, NULL);
    pthread_join(ui_thread, NULL);
    mutex_destroy();
    
    endwin();
    free(screen);
    fclose(tty);
}

void cleanup()
{
    QueueNode *tmp;
    QueueNode *head = cpuQueue->head;
    
    while (head)
    {
    	tmp = head;
    	head = head->next;
    
    	free(tmp);
    }
    
    a_free(&windowArena);
    a_free(&cpuArena);
    a_free(&memArena);
    a_free(&cpuGraphArena);
    a_free(&memoryGraphArena);
    a_free(&prcArena);
    a_free(&queueArena);
    a_free(&general);
    a_free(&cpuPointArena);
    a_free(&memPointArena);
    a_free(&stateArena);
}

static void * _ui_thread_run(void *arg)
{
    UIThreadArgs *args = (UIThreadArgs *)arg;
    
    run_ui(
    	args->di,
    	args->cpuQueue
    );
    
    return NULL;
}

static void * _io_thread_run(void *arg)
{
    IOThreadArgs *args = (IOThreadArgs *)arg;
    
    run_io(
	args->arenas,
    	args->cpuQueue,
	args->windows
    );
    
    return NULL;
}

static void _set_active_window(mt_Window *windows, mt_Window winToAdd)
{
    windows[mtopSettings->activeWindowCount++] = winToAdd; 
}

static u8 _get_option_after_flag_with_space(char **optarg, char **argv, u8 argc, u8 optind)
{
    if ((*optarg == NULL || strcmp((*optarg), "=") == 0) && optind < argc && argv[optind][0] != '-')
    {
	*optarg = argv[optind++];
    }

    return *optarg != NULL;
}

// put this into some sort of util file maybe?
void _handle_resize(int sig)
{
    if (sig == SIGWINCH) RESIZE = 1;
}
