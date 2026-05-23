#include "fs.h"
#include <dirent.h>
#include <sys/stat.h>

bool is_dir(const FileInfo *file_info) {
  FileType ft = file_info->file_type;
  if (ft == FILE_TYPE_DIRECTORY || ft == FILE_TYPE_LNK)
    return true;
  return false;
}

int cmp_dir(const void *a, const void *b) {
  FileInfo *a_ent = (FileInfo *)a;
  FileInfo *b_ent = (FileInfo *)b;

  if (is_dir(a_ent) && is_dir(b_ent)) {
    return strcmp_(a_ent->name, b_ent->name);
  } else if (!is_dir(a_ent) && is_dir(b_ent)) {
    return 1;
  } else if (is_dir(a_ent) && !is_dir(b_ent)) {
    return -1;
  } else if (!is_dir(a_ent) && !is_dir(b_ent)) {
    return strcmp_(a_ent->name, b_ent->name);
  }

  return -1;
}

static FileType classify_file(FileInfo *file_info) {
  if (file_info->mode == 0) {
    return FILE_TYPE_EXECUTABLE_BINARY;
  }

  if (S_ISLNK(file_info->mode)) {
    return FILE_TYPE_LNK;
  } else if (S_ISDIR(file_info->mode)) {
    return FILE_TYPE_DIRECTORY;
  }

  FileType file_type = FILE_TYPE_BINARY_DATA;

  FILE *file = fopen(file_info->path, "rb");
  if (!file)
    return file_type;

  unsigned char buf[512];
  size_t n = 0;
  if ((n = fread(buf, 1, sizeof(buf), file)) > 0) {
    if (buf[0] == 0x7F && buf[1] == 'E' && buf[2] == 'L' && buf[3] == 'F') {
      file_type = FILE_TYPE_EXECUTABLE_BINARY;
    } else if (buf[0] == '#' && buf[1] == '!') {
      return FILE_TYPE_EXECUTABLE_SCRIPT;
    }
  }

  fclose(file);
  return file_type;
}

static void fill_file_info(FileInfo *file_info, char *path, char *name) {
  struct stat st;
  snprintf(file_info->name, sizeof(file_info->name), "%s", name);
  append_path_segment(path, name, file_info->path, 2048);

  if (lstat(file_info->path, &st) != 0) {
    file_info->file_type = FILE_TYPE_BINARY_DATA;
    file_info->mod_time = 0;
    file_info->size = 0;
    file_info->mode = 0;
  }

  file_info->mode = st.st_mode;
  file_info->size = st.st_size;
  file_info->mod_time = st.st_mtim.tv_sec;
  file_info->file_type = classify_file(file_info);
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

  while ((fs_ent = readdir(root)) != NULL && count < buf_size) {
    char *name = fs_ent->d_name;

    if (strcmp(name, ".") == 0 ||
        (strcmp(path, "/") == 0 && strcmp(name, "..") == 0)) {
      continue;
    }

    FileInfo file_info = {0};
    fill_file_info(&file_info, path, name);

    buf[count] = file_info;
    count++;
  }

  closedir(root);
  return count;
}

void sort_dir(FileInfo *buf, int buf_size) {
  qsort_(buf, buf_size, sizeof(FileInfo), cmp_dir);
}
