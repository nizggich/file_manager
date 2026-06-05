#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HISTORY 50

typedef struct {
  char *paths[MAX_HISTORY];
  int count;
  int current;
} NavHistory;

void history_init(NavHistory *hist);

void history_free(NavHistory *hist);
int history_add(NavHistory *hist, char *path);
char *history_go_back(NavHistory *hist);
char *histoty_go_forward(NavHistory *hist);
char *history_get_current(NavHistory *hist);
