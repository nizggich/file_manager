#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HISTORY 50

typedef struct {
  char *paths[MAX_HISTORY];
  int count;
  int current;
} History;

void history_init(History *hist);

void history_free(History *hist);
int history_add(History *hist, char *path);
char *history_go_forward(History *hist);
char *history_go_back(History *hist);
char *history_get_current(History *hist);
