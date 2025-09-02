#ifndef UTIL_H
#define UTIL_H

#include "sorting.h"
#include "window.h"
#include "mt_type_defs.h"

u8 u_win_cmp_above(WindowData *cmp, WindowData *cur);
u8 u_win_cmp_below(WindowData *cmp, WindowData *cur);
u8 u_win_cmp_left(WindowData *cmp, WindowData *cur);
u8 u_win_cmp_right(WindowData *cmp, WindowData *cur);
LayoutOrientation u_get_orientation_for_layout(Layout layout);

static inline SortDirection util_get_sort_direction(
	ProcessListState *state,
	SortOrder sortOrder,
	SortDirection curSortDirection
)
{
	return (state->sortOrder == sortOrder && curSortDirection == ASC) ? DESC : ASC;
}

static inline SortDirection util_get_sort_direction_num(
	ProcessListState *state,
	SortOrder sortOrder,
	SortDirection curSortDirection
)
{
	return (state->sortOrder == sortOrder && curSortDirection == DESC) ? ASC : DESC;
}

#endif
