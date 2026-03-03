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


void append_path_segment(char *base_path, char *segment, char *buf, int buf_size) {
	if (!base_path || !segment || !buf || buf_size <= 0) {
		return;
	}

	int len = strlen(base_path);
	
	if (len == 0) {
		snprintf(buf, buf_size, "/%s", segment);
	}
	else if (base_path[len - 1] == '/') {
		snprintf(buf, buf_size, "%s%s", base_path, segment);
	}
	else {
		snprintf(buf, buf_size, "%s/%s", base_path, segment);
	}
}

int get_last_segment_start_index(char *path) {
	if (path == NULL) {
		return 0;
	}

	int i = strlen(path) - 1;	
	char *end_path = path + i;	
	char tmp;
	while ((tmp = *end_path) != '/') {
		end_path--;
		i--;
	}
	
	return i + 1;
}

void get_last_segment(char *path, char *buf, int buf_size) {

	int i = get_last_segment_start_index(path);
	int len = strlen(path) - i;
	strncpy(buf, path + i, len);
	buf[len] = '\0';
}

void substract_path_segment(char *base_path, char *buf, int buf_size) {
	if (!base_path  || !buf || strlen(base_path) > buf_size) {
		return;
	}

	int i = get_last_segment_start_index(base_path);
	
	if (i + 1 > buf_size) {
		return; 
	}

	strncpy(buf, base_path, i);

	if (i == 1) {
		buf[i] = '\0';
	 }
	else {
		buf[i - 1] = '\0';
	}	
	
}

int get_y(int item_index, int page_size) {
	int page = item_index / page_size; 
	return item_index - page * page_size + 3; 	
}
