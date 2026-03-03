#include "fs.h"

bool is_dir(const FileInfo *file_info) {
	if (file_info->mode != 0 && S_ISDIR(file_info->mode)) 
	{
		return true;
	}
	return false;
}


int cmp_dir(const void *a, const void *b) {
	FileInfo *a_ent = (FileInfo*) a;
	FileInfo *b_ent = (FileInfo*) b;
	
	if (is_dir(a_ent) && is_dir(b_ent)) {
		return strcmp_(a_ent->name, b_ent->name);
	}
	else if (!is_dir(a_ent) && is_dir(b_ent)) {
		return 1;
	}
	else if (is_dir(a_ent) && !is_dir(b_ent)) { 
		return -1;
	}
	else if (!is_dir(a_ent) && !is_dir(b_ent)) {
		return strcmp_(a_ent->name, b_ent->name);
	}	
}


bool is_text_file(const char *path) {

	FILE *f = fopen(path, "rb");
	if (!f) return false;

	unsigned char buffer[512];
	size_t n = fread(buffer, 1, sizeof(buffer), f);
	fclose(f);

	for (size_t i = 0; i < n; i++) {
		if(buffer[i] == '\0') {
			return false;
		}
	}
	
	return true;
}

FileType classify_file(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) {
	       return FILE_TYPE_BINARY_DATA;
	}	       	

	if (!S_ISREG(st.st_mode)) {
		return FILE_TYPE_DIRECTORY;
	}

	bool executable = (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0;
	bool is_text = is_text_file(path); 

	if (executable) {
		return is_text ? FILE_TYPE_EXECUTABLE_SCRIPT : FILE_TYPE_EXECUTABLE_BINARY;
	} else {
		return is_text ? FILE_TYPE_TEXT_PLAIN : FILE_TYPE_BINARY_DATA;
	}
}

int load_dir(char *path, FileInfo *buf, int buf_size) {
	if (!path || !buf || buf_size <= 0) {
		return -1;
	}

	DIR *root = opendir(path);
	struct dirent *fs_ent = NULL;

	struct stat sb;
	char dir_element_path[1024];
	
	int count = 0;
	
	while ((fs_ent = readdir(root)) != NULL && count < buf_size) 
	{
		char *name = fs_ent->d_name;
		int type = fs_ent->d_type;
		
		if (strcmp(name, ".") == 0 || (strcmp(path, "/") == 0 && strcmp(name, "..") == 0))
	       	{
			continue;
		}	
		
		FileInfo file_info = {0};
			
		snprintf(file_info.name, sizeof(file_info.name), "%s", name);
		append_path_segment(path, name, file_info.path, 1024);
		
		file_info.mode = sb.st_mode;
		file_info.type = type;	

		if (stat(file_info.path, &sb) == 0) {
			file_info.mode = sb.st_mode;
			file_info.size = sb.st_size;
			file_info.mod_time = sb.st_mtim.tv_sec;

		}

		file_info.file_type = classify_file(file_info.path);

		buf[count] = file_info;

		count++;
	}		

	closedir(root);

	return count;
}

void sort_dir(FileInfo *buf, int buf_size) {
	qsort_(buf, buf_size, sizeof(FileInfo), cmp_dir);
}
