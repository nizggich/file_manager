#include "../qsort/qsort.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum {
  FILE_TYPE_DIRECTORY,
  FILE_TYPE_LNK,
  FILE_TYPE_EXECUTABLE_SCRIPT,
  FILE_TYPE_EXECUTABLE_BINARY,
  FILE_TYPE_BINARY_DATA,
} FileType;

typedef struct {
  char name[128];
  char path[512];
  mode_t mode;
  off_t size;
  time_t mod_time;
  FileType file_type;
} FileInfo;

bool is_dir(const FileInfo *file_info);
int cmp_dir(const void *a, const void *b);
bool is_text_file(const char *path);
int load_dir(char *path, FileInfo *buf, int buf_size);
void sort_dir(FileInfo *buf, int buf_size);
