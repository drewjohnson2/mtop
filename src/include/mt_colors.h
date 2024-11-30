#ifndef MT_COLORS_H
#define MT_COLORS_H

#include <arena.h>

#include "mt_type_defs.h"

#define TO_NC_COLOR(rgbVal) (rgbVal = (unsigned short)((double)(rgbVal) * 3.92156))

typedef struct _mt_color 
{
	u16 red;
	u16 green;
	u16 blue;
} MT_Color;

typedef enum _ui_colors
{
#define DEF_COLORS(color, colEnumVal, memberName) color = colEnumVal,
#include "tables/color_table.h"
	MT_CLR_MAX = 255
#undef DEF_COLORS
} UI_Colors;

typedef enum _mt_color_pairs
{
#define DEF_PAIRS(pair,fg, bg) pair,
	MT_PAIR_BACKGROUND = 1,
#include "tables/pair_table.h"
	MT_PAIR_MAX
#undef DEF_PAIRS
} MT_Color_Pairs;

typedef struct _mt_ui_theme 
{
#define DEF_COLORS(color, colEnumVal, memberName) \
	MT_Color *memberName;
#include "tables/color_table.h"
#undef DEF_COLORS
} MT_UI_Theme;

MT_UI_Theme * import_colors(Arena *arena);
#endif
