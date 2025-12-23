#include <string.h>
#include <stdio.h>

int strcmp_(const char *a, const char *b);
void append_path_segment(char *base_path, char *segment, char *result, int rdesult_size);
void substract_path_segment(char *base_path, char *result, int result_size);
