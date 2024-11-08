#ifndef STARTUP_H
#define STARTUP_H

#include <pthread.h>

extern pthread_mutex_t ncursesLock;
extern pthread_cond_t ncursesDone;
extern int renderDone;

void run();

#endif
