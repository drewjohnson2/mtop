#include "../include/util/ui_utils.h"
#include "../include/monitor/proc_monitor.h"

typedef enum _sort_order 
{
	COMMAND
} SORT_ORDER;

int proc_name_compare(const void *a, const void *b)
{
	const PROC_STATS *x = *(PROC_STATS **)a;
	const PROC_STATS *y = *(PROC_STATS **)b;

	return strcmp(x->procName, y->procName);
}
