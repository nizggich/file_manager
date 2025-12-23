#include "utils.h"

int strcmp_(const char *a, const char *b) {

	int a_len = 0;
	int b_len = 0;

	while (1)
	{	
		if (*a == '\0') 
		{
			while (*b != '\0')
			{
				b_len++;
				b++;
			}
			break;
		}
		else if (*b == '\0') 
		{
			while (*a != '\0')
			{
				a_len++;
				a++;
			}
			break;
		}

		if (*a > *b)
			return 1;
		else if (*a < *b) 
			return -1;

	        a_len++;
		b_len++;
		
		a++;
		b++;
	}
	
	if (a_len == b_len) 
		return 0;
	else if (a_len > b_len)
		return 1;
	else 
		return -1;	
}


void append_path_segment(char *base_path, char *segment, char *result, int result_size) {
	if (!base_path || !segment || !result || result_size <= 0) {
		return;
	}

	int len = strlen(base_path);
	
	if (len == 0) {
		snprintf(result, result_size, "/%s", segment);
	}
	else if (base_path[len - 1] == '/') {
		snprintf(result, result_size, "%s%s", base_path, segment);
	}
	else {
		snprintf(result, result_size, "%s/%s", base_path, segment);
	}
}

void substract_path_segment(char *base_path, char *result, int result_size) {
	if (!base_path || strlen(base_path) > result_size) {
		return;
	}

	int i = strlen(base_path) - 1;	
	char *end_path = base_path + i;	
	char tmp;
	while ((tmp = *end_path) != '/') {
		end_path--;
		i--;
	}

	strncpy(result, base_path, i + 1);

	if (i == 0) {
		result[i + 1] = '\0';
	 }
	else {
		result[i] = '\0';
	}	
	
}

