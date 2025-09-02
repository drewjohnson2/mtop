#ifndef INPUT_H
#define INPUT_H

#include "window.h"

void read_normal_input(
    WINDOW *win,
    ProcessListState *state,
    UIData *ui
);
void read_arrange_input(UIData *ui);

#endif
