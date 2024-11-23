#include <assert.h>

#include "../include/ui_utils.h"
#include "../include/monitor.h"

typedef enum _sort_order 
{
	COMMAND
} SORT_ORDER;

int proc_name_compare(const void *a, const void *b)
{
	assert(a && b);

	const ProcessList *x = *(ProcessList **)a;
	const ProcessList *y = *(ProcessList **)b;

	return strcmp(x->procName, y->procName);
}

int proc_pid_compare(const void *a, const void *b)
{
	assert(a && b);

	const ProcessList *x = *(ProcessList **)a;
	const ProcessList *y = *(ProcessList **)b;

	return x->pid > y->pid;
}

int pid_search_func(const void *a, const void *b)
{
	assert(a && b);

	const ProcessList *x = *(ProcessList **)a;
	const ProcessList *y = *(ProcessList **)b;

	if (x->pid > y->pid)
	{
		return 1;
	}
	else if (x->pid < y->pid)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
