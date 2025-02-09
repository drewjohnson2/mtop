#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>

#include "../../include/mt_colors.h"

static MT_UI_Theme * _alloc_theme(Arena *arena);

void import_colors()
{
    FILE *f = fopen("./colors", "r");
    char buffer[255];
    
    if (!f) exit(1);
    
    Arena arena = a_new(512);
    MT_UI_Theme *theme = _alloc_theme(&arena);

#define DEF_COLORS(color, colEnumVal, memberName) \
    if (sscanf(buffer, #memberName " = [%hu, %hu, %hu]\n", &theme->memberName->red, 	\
    	&theme->memberName->green, &theme->memberName->blue) > 0) 			\
    { 											\
    	TO_NC_COLOR(theme->memberName->red); 						\
    	TO_NC_COLOR(theme->memberName->green); 						\
    	TO_NC_COLOR(theme->memberName->blue); 						\
											\
    	init_color(color, theme->memberName->red, 					\
    		theme->memberName->green, 						\
    		theme->memberName->blue); 						\
    }
    
    
    while (fgets(buffer, sizeof(buffer), f))
    {
#include "../../include/tables/color_table.h"
    }
#undef DEF_COLORS

    init_pair(MT_PAIR_BACKGROUND, MT_CLR_BACKGROUND, MT_CLR_BACKGROUND);

#define DEF_PAIRS(pair, fg, bg) init_pair(pair, fg, bg);
#include "../../include/tables/pair_table.h"
#undef DEF_PAIRS
    
    fclose(f);
    a_free(&arena);
}

static MT_UI_Theme * _alloc_theme(Arena *arena)
{
    MT_UI_Theme *theme = a_alloc(arena, sizeof(MT_UI_Theme), __alignof(MT_UI_Theme));

#define DEF_COLORS(color, colEnumVal, memberName) \
    theme->memberName = a_alloc(arena, sizeof(MT_Color), __alignof(MT_Color));
#include "../../include/tables/color_table.h"
#undef DEF_COLORS

    return theme;
}
