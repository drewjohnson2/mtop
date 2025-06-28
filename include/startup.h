#ifndef STARTUP_H
#define STARTUP_H

#include "mt_type_defs.h"
#include <pthread.h>

typedef struct _mtop_settings
{
    u8 transparencyEnabled;
} Settings;

extern volatile Settings *mtopSettings;

void run(u8 transparencyEnabled);
void cleanup();

#endif
