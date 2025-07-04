#define _GNU_SOURCE

#include <getopt.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>
#include <ctype.h>

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

typedef struct _ui_thread_args
{
    Arena *graphArena;
    Arena *memGraphArena;
    DisplayItems *di;
    volatile MemoryStats *memStats;
    ThreadSafeQueue *cpuQueue;
    ThreadSafeQueue *prcQueue;
    volatile ProcessInfoSharedData *prcInfoSD;
} UIThreadArgs;

typedef struct _io_thread_args
{
    Arena *cpuArena;
    Arena *prcArena;
    volatile MemoryStats *memStats;
    ThreadSafeQueue *cpuQueue;
    ThreadSafeQueue *prcQueue;
    volatile ProcessInfoSharedData *prcInfoSD;
} IOThreadArgs;

Arena windowArena;
Arena cpuArena;
Arena memArena; 
Arena cpuGraphArena;     
Arena memoryGraphArena;  
Arena prcArena;
Arena queueArena;
Arena general;

ThreadSafeQueue *cpuQueue;
ThreadSafeQueue *prcQueue;

volatile MemoryStats *memStats;
volatile ProcessInfoSharedData *prcInfoSD;

volatile Settings *mtopSettings;

static void * _ui_thread_run(void *arg);
static void * _io_thread_run(void *arg);
static void _set_active_window(mt_Window *windows, mt_Window winToAdd);

static mt_Window windows[3] = { WINDOW_ID_MAX, WINDOW_ID_MAX, WINDOW_ID_MAX};

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
    general = a_new(GENERAL_A_SZ);
    
    DisplayItems *di = init_display_items(&windowArena);
    
    cpuQueue = a_alloc(
    	&queueArena,
    	sizeof(ThreadSafeQueue),
    	__alignof(ThreadSafeQueue)
    );

    memStats = a_alloc(&memArena, sizeof(MemoryStats), __alignof(MemoryStats));
    
    prcQueue = a_alloc(
    	&queueArena,
    	sizeof(ThreadSafeQueue),
    	__alignof(ThreadSafeQueue)
    );

    prcInfoSD = (ProcessInfoSharedData *)a_alloc(
	&general,
	sizeof(ProcessInfoSharedData),
	__alignof(ProcessInfoSharedData)
    ); 
    prcInfoSD->info = a_alloc(&general, sizeof(ProcessInfo), __alignof(ProcessInfo));

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
	{ NULL, no_argument, NULL, 0 }
    };

    while((arg = getopt_long(argc, argv, "tcmpbv::", long_options, &option_index)) != -1)
    {
	switch (arg) 
    	{
    	    case 't':
		mtopSettings->transparencyEnabled = 1;
    	        break;
	    case 'c':
		mtopSettings->activeWindows[CPU_WIN] = 1;
		_set_active_window(windows, CPU_WIN);
		break;
	    case 'm':
		mtopSettings->activeWindows[MEMORY_WIN] = 1;
		_set_active_window(windows, MEMORY_WIN);
		break;
	    case 'p':
		mtopSettings->activeWindows[PRC_WIN] = 1;
		_set_active_window(windows, PRC_WIN);
		break;
	    case 'b':
		mtopSettings->orientation = HORIZONTAL;
		mtopSettings->layout = QUARTERS_TOP;
		break;
	    case 'v':
		mtopSettings->orientation = VERTICAL;

		if (optarg == NULL && optind < argc && argv[optind][0] != '-')
		{
		    optarg = argv[optind++];
		}

		if (optarg == NULL)
		{
		    mtopSettings->layout = QUARTERS_LEFT;
		    break;
		}
		
		if (strcmp(optarg, "right") == 0)
		{
		    mtopSettings->layout = QUARTERS_RIGHT;
		    break;
		}

		mtopSettings->layout = QUARTERS_LEFT;

		break;
    	    default:
    	        break;
    	}
    }

    if (windows[0] == WINDOW_ID_MAX && windows[1] == WINDOW_ID_MAX && windows[2] == WINDOW_ID_MAX)
    {
	mtopSettings->activeWindowCount = 3;

	windows[0] = CPU_WIN;
	windows[1] = MEMORY_WIN;
	windows[2] = PRC_WIN;

	mtopSettings->activeWindows[CPU_WIN] = 1;
	mtopSettings->activeWindows[MEMORY_WIN] = 1;
	mtopSettings->activeWindows[PRC_WIN] = 1;
    }

    if (mtopSettings->activeWindowCount == 2) mtopSettings->layout = DUO;
    else if (mtopSettings->activeWindowCount == 1) mtopSettings->layout = SINGLE;

    prcInfoSD->needsFetch = 0;
    prcInfoSD->pidToFetch = 0;
    
    init_ncurses(di->windows[CONTAINER_WIN], screen);
    init_window_dimens(di, windows);
    init_windows(di);
    
    UIThreadArgs uiArgs = 
    {
    	.graphArena = &cpuGraphArena,
    	.di = di,
    	.cpuQueue = cpuQueue,
	.memStats = memStats,
    	.memGraphArena = &memoryGraphArena,
    	.prcQueue = prcQueue,
	.prcInfoSD = prcInfoSD
    };
    
    IOThreadArgs ioArgs = 
    {
    	.cpuArena = &cpuArena,
    	.prcArena = &prcArena,
	.memStats = memStats,
    	.cpuQueue = cpuQueue,
	.prcQueue = prcQueue,
	.prcInfoSD = prcInfoSD
    };
    
    pthread_t ioThread;
    pthread_t ui_thread;

    // setup
    mutex_init();
    condition_init();
    pthread_create(&ioThread, NULL, _io_thread_run, (void *)&ioArgs);
    pthread_create(&ui_thread, NULL, _ui_thread_run, (void *)&uiArgs);

    // tear down
    pthread_join(ioThread, NULL);
    pthread_join(ui_thread, NULL);
    mutex_destroy();
    condition_destroy();
    
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
    
    head = prcQueue->head;

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
}

static void * _ui_thread_run(void *arg)
{
    UIThreadArgs *args = (UIThreadArgs *)arg;
    
    run_ui(
	args->graphArena,
    	args->memGraphArena,
    	args->di,
    	args->cpuQueue,
    	args->prcQueue,
	args->memStats,
	args->prcInfoSD
    );
    
    return NULL;
}

static void * _io_thread_run(void *arg)
{
    IOThreadArgs *args = (IOThreadArgs *)arg;
    
    run_io(
    	args->cpuArena,
    	args->prcArena,
    	args->cpuQueue,
    	args->prcQueue,
	args->memStats,
	args->prcInfoSD
    );
    
    return NULL;
}

static void _set_active_window(mt_Window *windows, mt_Window winToAdd)
{
    windows[mtopSettings->activeWindowCount++] = winToAdd; 
}


