#ifndef STARTUP_H
#define STARTUP_H

#include <pthread.h>

#include "mt_type_defs.h"

typedef enum _layout_orientation
{
    HORIZONTAL,
    VERTICAL,
} LayoutOrientation;

typedef enum _layout
{
    QUARTERS_LEFT,
    QUARTERS_RIGHT,
    QUARTERS_TOP,
    QUARTERS_BOTTOM,
    DUO,
    SINGLE
} Layout;

typedef struct _mtop_settings
{
    // only 3 active windows are possible, but I'm indexing with enum value
    // so I have to account for the container window. 
    u8 activeWindows[4];
    u8 transparencyEnabled;
    u8 activeWindowCount;
    Layout layout;
    LayoutOrientation orientation;
} Settings;

extern volatile Settings *mtopSettings;

void run(int argc, char **argv);
void cleanup();

#endif
