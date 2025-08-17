#include "../../include/menu.h"
#include "../../include/text.h"
#include "../../include/window.h"


static void _init_menu_idx(MenuItem **items, u8 itemCount);

void init_menu(
    UIData *ui,
    u8 isVisible,
    u8 itemCount,
    const char *windowTitle,
    void (*onSelect)(UIData *, MenuItemValue),
    void (*initMenuItems)(MenuItem **)
)
{
    WindowData *container = ui->windows[CONTAINER_WIN];
    WindowData *menu = ui->windows[STAT_TYPE_WIN];

    ui->menu->isVisible = isVisible;
    ui->menu->menuItemCount = itemCount;
    ui->menu->on_select = onSelect; 
    ui->menu->windowTitle = windowTitle;

    size_floating_win(container, menu, ui->menu->menuItemCount + 2, FLOAT_WIN_DEFAULT_W(container));
    menu->window = subwin(container->window, menu->wHeight, menu->wWidth, menu->windowY, menu->windowX);

    initMenuItems(ui->menu->items);
    _init_menu_idx(ui->menu->items, itemCount);
}

void init_stat_menu_items(MenuItem **items) 
{
#define DEF_MENU_ITEMS(idx, txt, win) 				\
    items[idx]->displayString = text(txt); 			\
    items[idx]->returnValue.windowType = win; 			\
    items[idx]->isSelected = false; 				\
    items[idx]->isHidden = mtopSettings->activeWindows[win]; 	

#include "../../include/tables/stat_menu_item_table.h"
#undef DEF_MENU_ITEMS
}

void init_layout_menu_items(MenuItem **items) 
{
#define DEF_MENU_ITEMS(idx, txt, lyt) 		\
    items[idx]->displayString = text(txt); 	\
    items[idx]->returnValue.layout = lyt; 	\
    items[idx]->isSelected = false;		\
    items[idx]->isHidden = false;		

#include "../../include/tables/layout_menu_items.h"
#undef DEF_MENU_ITEMS
}

void init_orienation_menu_items(MenuItem **items)
{
#define DEF_MENU_ITEMS(idx, txt, ort) 		\
    items[idx]->displayString = text(txt); 	\
    items[idx]->returnValue.orientation = ort; 	\
    items[idx]->isSelected = false;		\
    items[idx]->isHidden = false;		

#include "../../include/tables/orientation_menu_items.h"
#undef DEF_MENU_ITEMS
}

void reset_menu_idx(MenuItem **items, u8 itemCount) 
{
    for (size_t i = 0; i < itemCount; i++) items[i]->isSelected = false;
}

void select_next_menu_item(MenuItem **items, u8 winCount) 
{
    s8 selectedIdx = -1;

    for (s8 i = 0; i < winCount; i++) 
    {
	if (items[i]->isSelected) {
	    selectedIdx = i;
	    break;
	}
    }

    for (s8 i = selectedIdx + 1;; i++) 
    {
	if (i > winCount - 1) 
	{
	    i = -1;
	    continue;
	} 
	else if (items[i]->isHidden) continue;

	items[i]->isSelected = true;
	items[selectedIdx]->isSelected = false;
	break;
    }
}

MenuItemValue get_menu_selection(MenuItem **items, u8 itemCount) 
{
    for (size_t i = 0; i < itemCount; i++) 
    {
	if (items[i]->isSelected) 
	{
	    items[i]->isSelected = false;

	    return items[i]->returnValue;
	}
    }

    return items[0]->returnValue;
}

void display_menu_options(UIData *ui) 
{
    WindowData *statTypeWin = ui->windows[STAT_TYPE_WIN];
    u8 titlePosY = 0;
    const u8 titlePosX = (statTypeWin->wWidth / 2) - (strlen(text(TXT_ADD_WINDOW)) / 2);
    const u8 numPosX = 4;
    const u8 valPosX = 7;
    size_t optionNumber = 1;

    werase(statTypeWin->window);
    SET_COLOR(statTypeWin->window, MT_PAIR_BOX);
    box(statTypeWin->window, 0, 0);
    PRINTFC(statTypeWin->window, titlePosY++, titlePosX, "%s", ui->menu->windowTitle, MT_PAIR_CTRL_TXT);

    for (size_t i = 0; i < ui->menu->menuItemCount; i++) 
    {
	MenuItem *item = ui->menu->items[i];
	MT_Color_Pairs pair = MT_PAIR_PRC_UNSEL_TEXT;
	
	if (item->isHidden) continue;
	
	if (item->isSelected) 
	{
	    pair = MT_PAIR_PRC_SEL_TEXT;
	
	    for (size_t y = valPosX - 1; y < (size_t)(statTypeWin->wWidth - 4); y++)
		PRINTFC(statTypeWin->window, titlePosY, y, "%c", ' ', pair);
	}
	
	PRINTFC(statTypeWin->window, titlePosY, numPosX, "%zu.", optionNumber++, MT_PAIR_CPU_HEADER);
	PRINTFC(statTypeWin->window, titlePosY++, valPosX, "%s", item->displayString, pair);
    }
}

void handle_add_window(UIData *ui, MenuItemValue selection) 
{
    mt_Window winToAdd = selection.windowType;

    if (winToAdd == WINDOW_ID_MAX) return;

    add_win(ui, winToAdd);

    ui->menu->isVisible = false;
}

void handle_change_layout(UIData *ui, MenuItemValue selection)
{
    mtopSettings->layout = selection.layout;

    init_window_dimens(ui);
    reinit_window(ui);

    ui->menu->isVisible = false;
}

void handle_change_duo_orientation(UIData *ui, MenuItemValue selection)
{
    mtopSettings->orientation = selection.orientation;

    init_window_dimens(ui);
    reinit_window(ui);

    ui->menu->isVisible = false;
}

static void _init_menu_idx(MenuItem **items, u8 itemCount) 
{
    for (size_t i = 0; i < itemCount; i++) 
    {
	MenuItem *item = items[i];

	if (item->isHidden) continue;

	item->isSelected = true;

	return;
    }
}
