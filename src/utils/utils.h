#include <stdio.h>
#include <string.h>

int strcmp_(const char *a, const char *b);
void append_path_segment(const char *base_path, char *segment, char *buf,
                         int buf_size);
void substract_path_segment(char *base_path, char *buf, int buf_size);
int get_last_segment_start_index(char *path);
void get_last_segment(char *path, char *buf, int buf_size);
int get_y(int item_index, int page_size);
