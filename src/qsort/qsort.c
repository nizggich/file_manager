#include "qsort.h"

int cmp_str(const void *a, const void *b) {
	const char *a_ptr = *(const char **)a;
	const char *b_ptr = *(const char **)b;
	
	return strcmp_(a_ptr, b_ptr);
}

int cmp_int(const void *a, const void *b) {
	int va = *(int *)a;
	int vb = *(int *)b;

	if (va > vb)
		 return 1;
	else if (va < vb)
		return -1;
	else
		return 0;	
}

void swap(void *a, void *b, size_t type_size) {
	char *pa = (char*)a;
	char *pb = (char*)b;

	for (int i = 0; i < type_size; i++) 
	{
		char tmp = pa[i];
		pa[i] = pb[i];
		pb[i] = tmp;
	}

}

void qsort_(void *base, int size, size_t type_size, comparator cmp) {
	if (size <= 1)
		return;

	char *arr = (char*)base;
	char *pivot = arr + ((size - 1 )/ 2) * type_size;
	
	int left = 0;
	int right = size - 1;

	char *left_ptr = arr + type_size * left;
	char *right_ptr = arr + type_size * right;

	while (1) 
	{		
		while (cmp(left_ptr, pivot) < 0)
	       	{ 	
			left++;	
			left_ptr += type_size; 
		}
		

		while (cmp(right_ptr, pivot) > 0)	
		{
			right--;	
			right_ptr -= type_size;
		}
	

		if (left >= right) 	
			break;

		if (left_ptr == pivot)
		{
			pivot = right_ptr;
		} 
		else if (right_ptr == pivot)
		{
			pivot = left_ptr;
		}

		swap(left_ptr, right_ptr, type_size);


		left++;
		right--;

		left_ptr += type_size;
		right_ptr -= type_size;


	}

	qsort_(arr, right + 1, type_size, cmp);
	qsort_(arr + (type_size * (right + 1)), size - (right + 1), type_size, cmp);

	
}		

