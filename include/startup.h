#ifndef STARTUP_H
#define STARTUP_H

#include <pthread.h>

#include "mt_type_defs.h"
#include "window.h"

typedef struct _mtop_settings
{
    u8 transparencyEnabled;
    u8 allWindowsActive;
    // only 3 active windows are possible, but I'm indexing with enum value
    // so I have to account for the container window. 
    u8 activeWindows[4];
    u8 activeWindowCount;
} Settings;

extern volatile Settings *mtopSettings;

void run(int argc, char ** argv);
void cleanup();

#endif
