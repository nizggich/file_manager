#include "fs.h"
#include <dirent.h>
#include <sys/stat.h>

int cmp_dir(const void *a, const void *b) {
  FileInfo *a_ent = (FileInfo *)a;
  FileType a_type = a_ent->file_type;

  FileInfo *b_ent = (FileInfo *)b;
  FileType b_type = b_ent->file_type;

  if (is_dir(a_type) && is_dir(b_type)) {
    return strcmp_(a_ent->name, b_ent->name);
  } else if (!is_dir(a_type) && is_dir(b_type)) {
    return 1;
  } else if (is_dir(a_type) && !is_dir(b_type)) {
    return -1;
  } else if (!is_dir(a_type) && !is_dir(b_type)) {
    return strcmp_(a_ent->name, b_ent->name);
  }

  return -1;
}

LinkTargetType classify_link(const char *path) {
  struct stat st;
  if (lstat(path, &st) != 0 || !S_ISLNK(st.st_mode)) {
    return LINK_TARGET_UNKNOWN;
  }

  if (stat(path, &st) == 0) {
    if (S_ISDIR(st.st_mode))
      return LINK_TARGET_DIR;
    return LINK_TARGET_OTHER;
  }

  return LINK_TARGET_BROKEN;
}

static BinaryFileType classify_bin_file(const char *path) {
  struct stat st;
  BinaryFileType file_type = BIN_FILE_UNKNOW;

  if (lstat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
    return file_type;
  }

  FILE *file = fopen(path, "rb");
  if (!file)
    return file_type;

  unsigned char buf[512];
  size_t n = 0;
  if ((n = fread(buf, 1, sizeof(buf), file)) > 0) {
    if (buf[0] == 0x7F && buf[1] == 'E' && buf[2] == 'L' && buf[3] == 'F') {
      file_type = BIN_FILE_ELF;
    } else if (buf[0] == '#' && buf[1] == '!') {
      file_type = BIN_FILE_EXEC_SCRIPT;
    }
  }

  fclose(file);
  return file_type;
}

FileType classify_file(const char *path) {
  if (path == NULL) {
    return FILE_TYPE_EXECUTABLE_BINARY;
  }

  struct stat st;
  if (lstat(path, &st) != 0) {
    return FILE_TYPE_UNKNOWN;
  }

  if (S_ISDIR(st.st_mode))
    return FILE_TYPE_DIRECTORY;
  if (S_ISLNK(st.st_mode))
    return (int)classify_link(path);
  if (S_ISREG(st.st_mode))
    return (int)classify_bin_file(path);

  return FILE_TYPE_BINARY_DATA;
}

static int fill_file_info(FileInfo *file_info, char *path, char *name) {
  struct stat st;
  snprintf(file_info->name, sizeof(file_info->name), "%s", name);
  append_path_segment(path, name, file_info->path, 2048);

  if (lstat(file_info->path, &st) != 0) {
    return -1;
  }

  file_info->mode = st.st_mode;
  file_info->size = st.st_size;
  file_info->mod_time = st.st_mtim.tv_sec;
  file_info->file_type = classify_file(file_info->path);

  return 0;
}

int load_dir(char *path, FileInfo *buf, int buf_size) {
  if (!path || !buf || buf_size <= 0) {
    return -1;
  }

  DIR *root = opendir(path);
  struct dirent *fs_ent = NULL;

  int count = 0;

  while ((fs_ent = readdir(root)) != NULL && count < buf_size) {
    char *name = fs_ent->d_name;

    if (strcmp(name, ".") == 0 ||
        (strcmp(path, "/") == 0 && strcmp(name, "..") == 0)) {
      continue;
    }

    FileInfo file_info = {0};
    int status = fill_file_info(&file_info, path, name);

    if (status == 0) {
      buf[count] = file_info;
      count++;
    }
  }

  closedir(root);
  return count;
}

void sort_dir(FileInfo *buf, int buf_size) {
  qsort_(buf, buf_size, sizeof(FileInfo), cmp_dir);
}
