#ifndef STARTUP_H
#define STARTUP_H

#include <pthread.h>
#include <arena.h>

#include "mt_type_defs.h"

typedef enum
{
    HORIZONTAL,
    VERTICAL,
} LayoutOrientation;

typedef enum
{
    QUARTERS_LEFT,
    QUARTERS_RIGHT,
    QUARTERS_TOP,
    QUARTERS_BOTTOM,
    DUO,
    SINGLE
} Layout;

typedef struct
{
    // only 3 active windows are possible, but I'm indexing with enum value
    // so I have to account for the container window. 
    u8 activeWindows[4];
    u8 transparencyEnabled;
    u8 activeWindowCount;
    Layout layout;
    LayoutOrientation orientation;
} Settings;

typedef struct
{
    Arena *windowArena;
    Arena *cpuArena;
    Arena *memArena; 
    Arena *cpuGraphArena;     
    Arena *memoryGraphArena;  
    Arena *prcArena;
    Arena *queueArena;
    Arena *general;
    Arena *cpuPointArena;
    Arena *memPointArena;
    Arena *stateArena;
} mtopArenas;

extern volatile Settings *mtopSettings;

void run(int argc, char **argv);
void cleanup();

#endif
