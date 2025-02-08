#ifndef SORTING_H
#define SORTING_H

#define EPSILON 1e-9

typedef enum _sort_dir 
{
    ASC,
    DESC
} SortDirection;

typedef enum _sort_order
{
    CPU,
    MEM,
    PRC_NAME,
    PID
} SortOrder;

extern SortDirection sortDirection;

int prc_name_compare(const void *a, const void *b);
int prc_pid_compare(const void *a, const void *b);
int vd_name_compare_func(const void *a, const void *b);
int vd_pid_compare_func(const void *a, const void *b);
int vd_cpu_compare_func(const void *a, const void *b);
int vd_mem_compare_func(const void *a, const void *b);

#endif
