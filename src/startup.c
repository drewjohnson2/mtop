#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>
#include <unistd.h>

#include "../include/startup.h"
#include "../include/monitor.h"
#include "../include/thread_safe_queue.h"
#include "../include/window.h"
#include "../include/thread.h"

typedef struct _ui_thread_args
{
    Arena *graphArena;
    Arena *memGraphArena;
    DisplayItems *di;
    ThreadSafeQueue *cpuQueue;
    ThreadSafeQueue *memQueue;
    ThreadSafeQueue *prcQueue;
    volatile ProcessInfoSharedData *prcInfoSD;
} UIThreadArgs;

typedef struct _io_thread_args
{
    Arena *cpuArena;
    Arena *memArena;
    Arena *prcArena;
    ThreadSafeQueue *cpuQueue;
    ThreadSafeQueue *memQueue;
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
ThreadSafeQueue *memoryQueue;
ThreadSafeQueue *prcQueue;

MemoryStats *memStats;

volatile ProcessInfoSharedData *prcInfoSD;

static void * _ui_thread_run(void *arg);
static void * _io_thread_run(void *arg);

void run() 
{
    FILE *tty = fopen("/dev/tty", "r+");
    SCREEN *screen = newterm(NULL, tty, tty);

    // If program starts crashing randomly after
    // any period of time it's probably time to roll
    // back to a different commit on this allocation block.
    windowArena = a_new(256);
    cpuArena = a_new(sizeof(CpuStats) + 8);
    memArena = a_new(sizeof(MemoryStats) + 8);
    cpuGraphArena = a_new(sizeof(GraphData));     
    memoryGraphArena = a_new(sizeof(GraphData));  
    prcArena = a_new(MAX_PROCS * sizeof(ProcessList *));
    queueArena = a_new(sizeof(ThreadSafeQueue));
    general = a_new(256 * 20);
    
    DisplayItems *di = init_display_items(&windowArena);
    
    cpuQueue = a_alloc(
    	&queueArena,
    	sizeof(ThreadSafeQueue),
    	__alignof(ThreadSafeQueue)
    );

    memStats = a_alloc(&general, sizeof(MemoryStats), __alignof(MemoryStats));
    
    memoryQueue = a_alloc(
    	&queueArena,
    	sizeof(ThreadSafeQueue),
    	__alignof(ThreadSafeQueue)
    );
    
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

    prcInfoSD->needsFetch = 0;
    prcInfoSD->pidToFetch = 0;
    
    init_ncurses(di->windows[CONTAINER_WIN], screen);
    init_window_dimens(di);
    init_windows(di);
    
    UIThreadArgs uiArgs = 
    {
    	.graphArena = &cpuGraphArena,
    	.di = di,
    	.cpuQueue = cpuQueue,
    	.memGraphArena = &memoryGraphArena,
    	.memQueue = memoryQueue,
    	.prcQueue = prcQueue,
	.prcInfoSD = prcInfoSD
    };
    
    IOThreadArgs ioArgs = 
    {
    	.cpuArena = &cpuArena,
    	.memArena = &memArena,
    	.prcArena = &prcArena,
    	.cpuQueue = cpuQueue,
    	.memQueue = memoryQueue,
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
    QueueNode *head = memoryQueue->head;
    
    while (head)
    {
	tmp = head;
    	head = head->next;
    
    	free(tmp);
    }
    
    head = cpuQueue->head;
    
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
    	args->memQueue,
    	args->prcQueue,
	args->prcInfoSD
    );
    
    return NULL;
}

static void * _io_thread_run(void *arg)
{
    IOThreadArgs *args = (IOThreadArgs *)arg;
    
    run_io(
    	args->cpuArena,
    	args->memArena,
    	args->prcArena,
    	args->cpuQueue,
    	args->memQueue,
    	args->prcQueue,
	args->prcInfoSD
    );
    
    return NULL;
}
