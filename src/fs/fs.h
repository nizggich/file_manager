#include "../qsort/qsort.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum {
  LINK_TARGET_UNKNOWN = 1,
  LINK_TARGET_DIR,
  LINK_TARGET_OTHER,
  LINK_TARGET_BROKEN,
} LinkTargetType;

typedef enum {
  BIN_FILE_EXEC_SCRIPT = 5,
  BIN_FILE_ELF,
  BIN_FILE_UNKNOW,
} BinaryFileType;

typedef enum {
  FILE_TYPE_DIRECTORY = 0,
  FILE_TYPE_LNK_UNKNOWN = LINK_TARGET_UNKNOWN,
  FILE_TYPE_LNK_TO_DIR = LINK_TARGET_DIR,
  FILE_TYPE_LNK_OTHER = LINK_TARGET_OTHER,
  FILE_TYPE_LNK_BROKEN = LINK_TARGET_BROKEN,
  FILE_TYPE_EXECUTABLE_SCRIPT = BIN_FILE_EXEC_SCRIPT,
  FILE_TYPE_EXECUTABLE_BINARY = BIN_FILE_ELF,
  FILE_TYPE_BINARY_DATA = BIN_FILE_UNKNOW,
  FILE_TYPE_UNKNOWN
} FileType;

typedef struct {
  char name[128];
  char path[512];
  mode_t mode;
  off_t size;
  time_t mod_time;
  FileType file_type;
} FileInfo;

static inline bool is_binary(FileType file_type) {
  switch (file_type) {
  case FILE_TYPE_BINARY_DATA:
  case FILE_TYPE_EXECUTABLE_BINARY:
  case FILE_TYPE_EXECUTABLE_SCRIPT:
    return true;
  default:
    return false;
  }
}

static inline bool is_dir(FileType file_type) {
  if (file_type == FILE_TYPE_DIRECTORY || file_type == FILE_TYPE_LNK_TO_DIR)
    return true;
  return false;
}

int cmp_dir(const void *a, const void *b);
bool is_text_file(const char *path);
FileType classify_file(const char *path);
int load_dir(char *path, FileInfo *buf, int buf_size);
void sort_dir(FileInfo *buf, int buf_size);
