#ifndef MENU_H
#define MENU_H

#include "window.h"

void init_menu(
    UIData *ui,
    u8 isVisible,
    u8 itemCount,
    const char *windowTitle,
    void (*onSelect)(UIData *, MenuItemValue),
    void (*initMenuItems)(MenuItem **)
);
void init_stat_menu_items(MenuItem **items);
void init_layout_menu_items(MenuItem **items);
void init_orienation_menu_items(MenuItem **items);
void reset_menu_idx(MenuItem **items, u8 itemCount);
void select_previous_menu_item(MenuItem **items, u8 itemCount);
void select_next_menu_item(MenuItem **items, u8 itemCount);
void display_menu_options(UIData *ui);
void handle_add_window(UIData *ui, MenuItemValue selection);
void handle_change_layout(UIData *ui, MenuItemValue selection);
void handle_change_duo_orientation(UIData *ui, MenuItemValue selection);
MenuItemValue get_menu_selection(MenuItem **items, u8 itemCount);

#endif
