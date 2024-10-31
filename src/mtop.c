#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <arena.h>

#include "../include/window/window_setup.h"

int main() 
{
	FILE *tty = fopen("/dev/tty", "r+");
	SCREEN *screen = newterm(NULL, tty, tty);

	Arena windowArena = a_new(2048);
	DISPLAY_ITEMS *di = init_display_items(&windowArena);

	init_ncurses(di->windows[CONTAINER_WIN], screen);
	init_window_dimens(di);
	init_windows(di);

	wgetch(di->windows[CONTAINER_WIN]->window);

	endwin();

	free(screen);
	a_free(&windowArena);

	fclose(tty);
	return 0;
}
