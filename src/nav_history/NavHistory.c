#include "NavHistory.h"

void history_init(NavHistory *hist) {
  hist->count = 0;
  hist->current = 0;

  for (int i = 0; i < MAX_HISTORY; i++) {
    hist->paths[i] = NULL;
  }
}

void history_free(NavHistory *hist) {
  for (int i = 0; i < hist->count; i++) {
    if (hist->paths[i] != NULL) {
      free(hist->paths[i]);
    }
  }

  hist->count = 0;
  hist->current = 0;
}

int history_add(NavHistory *hist, char *path) {

  if (!path || !hist) {
    return -1;
  }

  if (hist->count == MAX_HISTORY) {
    free(hist->paths[0]);
    memmove(&hist[0], &hist[1], MAX_HISTORY - 1);

    hist->count = MAX_HISTORY - 1;
    hist->current = hist->count - 1;
  }

  hist->count++;
  hist->current = hist->count - 1;
  hist->paths[hist->current] = strdup(path);

  if (!hist->paths[hist->current]) {
    return -1;
  }

  return 0;
}

char *history_go_forward(NavHistory *hist) {
  if (!hist || hist->current < 0 || hist->current == hist->count - 1) {
    return NULL;
  }

  if (hist->current == MAX_HISTORY - 1) {
    return hist->paths[hist->current];
  }

  return hist->paths[++hist->current];
}

char *history_go_back(NavHistory *hist) {
  if (!hist || hist->current <= 0) {
    return NULL;
  }

  return hist->paths[--hist->current];
}

char *history_get_current(NavHistory *hist) {
  if (hist->current >= 0 && hist->current < hist->count) {
    return hist->paths[hist->current];
  }

  return NULL;
}
