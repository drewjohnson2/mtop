#include <arena.h>
#include <stdio.h>

#include "../include/mt_colors.h"

MT_UI_Theme * import_colors(Arena *arena);
static MT_UI_Theme * _alloc_theme(Arena *arena);

MT_UI_Theme * import_colors(Arena *arena)
{
	FILE *f = fopen("../../colors", "r");
	char buffer[255];

	if (!f) return NULL;

	MT_UI_Theme *theme = _alloc_theme(arena);

#define DEF_COLORS(color, colEnumVal, memberName) \
	sscanf(buffer, #memberName " = [%hhu, %hhu, %hhu]\n", &theme->memberName->red, \
		&theme->memberName->green, &theme->memberName->blue);

	while (fgets(buffer, sizeof(buffer), f))
	{
#include "../include/tables/color_table.h"
	}
#undef DEF_COLORS

	return theme;
}

static MT_UI_Theme * _alloc_theme(Arena *arena)
{
	MT_UI_Theme *theme = a_alloc(arena, sizeof(MT_UI_Theme), __alignof(MT_UI_Theme));

#define DEF_COLORS(color, colEnumVal, memberName) \
	theme->memberName = a_alloc(arena, sizeof(MT_Color), __alignof(MT_Color));
#include "../include/tables/color_table.h"
#undef DEF_COLORS

	return theme;
}




