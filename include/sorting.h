#ifndef SORTING_H
#define SORTING_H

#define EPSILON 1e-9

typedef enum
{
    ASC,
    DESC
} SortDirection;

typedef enum
{
    CPU,
    MEM,
    PRC_NAME,
    PID
} SortOrder;

extern SortDirection sortDirection;

int prc_name_compare(const void *a, const void *b);
int prc_pid_compare(const void *a, const void *b);
int prc_tracked_stat_cmp(const void *a, const void *b);
int prc_pid_compare_without_direction_fn(const void *a, const void *b);
int prc_find_by_pid_compare_fn(const void *key, const void *a);
int vd_name_compare_fn(const void *a, const void *b);
int vd_pid_compare_fn(const void *a, const void *b);
int vd_cpu_compare_fn(const void *a, const void *b);
int vd_mem_compare_fn(const void *a, const void *b);
int vd_find_by_pid_compare_fn(const void *key, const void *a);
int vd_pid_compare_without_direction_fn(const void *a, const void *b);

#endif
