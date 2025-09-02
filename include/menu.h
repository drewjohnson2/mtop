#ifndef MENU_H
#define MENU_H

#include "window.h"

#define LAYOUT_COUNT 4
#define ORIENTATION_COUNT 2
#define ITEM_PADDING 2
#define ITEM_NUM_START_X 4
#define ITEM_TEXT_START_X 7

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
void menu_reset_idx(MenuItem **items, u8 itemCount);
void menu_select_previous_item(MenuItem **items, u8 itemCount);
void menu_select_next_item(MenuItem **items, u8 itemCount);
void menu_display_options(UIData *ui);
void menu_handle_add_window(UIData *ui, MenuItemValue selection);
void menu_handle_change_layout(UIData *ui, MenuItemValue selection);
void menu_handle_change_duo_orientation(UIData *ui, MenuItemValue selection);
MenuItemValue menu_get_selection(MenuItem **items, u8 itemCount);

#endif
