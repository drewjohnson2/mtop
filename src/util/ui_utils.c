#include "../include/ui_utils.h"
#include "../include/monitor.h"

typedef enum _sort_order 
{
	COMMAND
} SORT_ORDER;

int proc_name_compare(const void *a, const void *b)
{
	const ProcessStats *x = *(ProcessStats **)a;
	const ProcessStats *y = *(ProcessStats **)b;

	return strcmp(x->procName, y->procName);
}
