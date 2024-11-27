#include <assert.h>

#include "../include/ui_utils.h"
#include "../include/monitor.h"

typedef enum _sort_order 
{
	COMMAND
} SORT_ORDER;

int prc_name_compare(const void *a, const void *b)
{
	assert(a && b);

	const ProcessList *x = *(ProcessList **)a;
	const ProcessList *y = *(ProcessList **)b;

	return strcmp(x->procName, y->procName);
}

int prc_pid_compare(const void *a, const void *b)
{
	assert(a && b);

	const ProcessList *x = *(ProcessList **)a;
	const ProcessList *y = *(ProcessList **)b;

	return x->pid - y->pid;
}
