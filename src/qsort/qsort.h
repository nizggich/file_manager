#include <stdio.h>
#include <stdlib.h>
#include "../utils/utils.h"

typedef int (*comparator)(const void *a, const void *b);

void swap(void *a, void *b, size_t type_size);
int cmp_int(const void *a, const void *b);
int cmp_str(const void *a, const void *b);
int cmp_fs_entitites(const void *a, const void *b);
void qsort_(void *base, int size, size_t type_size, comparator cmp);
	
