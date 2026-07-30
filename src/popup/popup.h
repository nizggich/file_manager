#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define POPUP_DEF_HEIGHT 4

typedef struct {
  char *title;
  WINDOW *win;
  int height;
  int width;
  int x;
  int y;
  void (*handle_input)(WINDOW *win, char *buf, int size);
  void (*wait_input)(WINDOW *win);
} BaseWin;

typedef struct {
  BaseWin base_win;

} Promt;

typedef struct {
  BaseWin base_win;

} Dialog;

typedef struct {
  char *title;
  int x;
  int y;
  int height;
  int width;
} BaseParam;

typedef struct {
  char *accept;
  char *decline;
} DialogParam;

Promt *popup_create_promt(BaseParam *param);
Dialog *popup_create_dialog(BaseParam *param, DialogParam *dparam);
void popup_gen_def_param(char *title, BaseParam *bp);
void popup_scale(BaseWin *bw);
