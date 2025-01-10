#ifndef UI_UTILS_
#define UI_UTILS_

#define EPSILON 1e-9

int prc_name_compare(const void *a, const void *b);
int prc_pid_compare(const void *a, const void *b);
int vd_name_compare_func(const void *a, const void *b);
int vd_pid_compare_func(const void *a, const void *b);
int vd_cpu_compare_func(const void *a, const void *b);
int vd_mem_compare_func(const void *a, const void *b);

#endif
