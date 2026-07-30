#include "popup.h"

extern int term_width, term_height;

static void handle_promt_input(WINDOW *win, char *input_buf, int size) {
  noecho();

  int win_width = getmaxx(win);

  int ch = 0;
  int i = -1;
  int curs_pos = 0;

  while ((ch = wgetch(win)) != '\n' && i != size - 2) {
    if (ch == 27) {
      input_buf[0] = '\0';
      return;
    }
    if (ch == 127) {
      if (i >= 0 && curs_pos > 0) {
        input_buf[i] = '\0';
        mvwprintw(win, 2, curs_pos, "%c", ' ');
        i--;
        curs_pos--;
      } else if (i > 0 && curs_pos <= 0) {
        int start = i - (win_width - 3);
        char prev_str[win_width - 2];
        strncpy(prev_str, &input_buf[start], win_width - 2);
        prev_str[win_width - 2] = '\0';
        mvwprintw(win, 2, 1, "%s", prev_str);
        curs_pos = win_width - 2;
      }
      continue;
    }

    if (curs_pos == win_width - 2) {
      mvwhline(win, 2, 1, ' ', win_width - 1);
      refresh();
      curs_pos = 0;
    }

    i++;
    curs_pos++;
    input_buf[i] = ch;
    mvwprintw(win, 2, curs_pos, "%c", ch);

    box(win, 0, 0);
  }

  input_buf[i + 1] = '\0';
}

static void handle_dialog_input(WINDOW *win, char *input_buf, int size) {
  char ch = wgetch(win);
  input_buf[0] = ch;
}

static void wait_input(WINDOW *win) {
  int ch;

  while ((ch = wgetch(win)) != '\n' && ch != 27 && ch != 'q') {
  }
}

static WINDOW *create_window(BaseParam *param) {
  char *title = param->title;
  int height = param->height;
  int width = param->width;
  int x = param->x;
  int y = param->y;

  int title_len = strlen(param->title);
  int borders = 2;

  WINDOW *win = newwin(height, width, y, x);

  int title_pos = (width / 2) - title_len / 2;
  if (title_pos < 1) {
    title_pos = 1;
  }

  int title_end = width - borders;
  if (title_len > title_end) {
    title[title_end] = '\0';
  }

  box(win, 0, 0);
  mvwprintw(win, 1, title_pos, "%s", title);

  return win;
}

Promt *popup_create_promt(BaseParam *param) {

  WINDOW *promt_win = create_window(param);

  Promt *pw = calloc(1, sizeof(Promt));
  pw->base_win.title = param->title;
  pw->base_win.win = promt_win;
  pw->base_win.x = param->x;
  pw->base_win.y = param->y;
  pw->base_win.width = param->width;
  pw->base_win.height = param->height;
  pw->base_win.handle_input = handle_promt_input;
  pw->base_win.wait_input = wait_input;

  return pw;
}

Dialog *popup_create_dialog(BaseParam *param, DialogParam *dparam) {
  WINDOW *dialog_win = create_window(param);
  int x1 = param->width / 4;
  int x2 = param->width - x1;
  mvwprintw(dialog_win, param->height - 2, x1, "%s", dparam->accept);
  mvwprintw(dialog_win, param->height - 2, x2, "%s", dparam->decline);

  Dialog *d = calloc(1, sizeof(Dialog));
  d->base_win.title = param->title;
  d->base_win.win = dialog_win;
  d->base_win.x = param->x;
  d->base_win.y = param->y;
  d->base_win.width = param->width;
  d->base_win.height = param->height;
  d->base_win.handle_input = handle_dialog_input;

  return d;
}

void popup_gen_def_param(char *title, BaseParam *bp) {
  int borders = 2;
  int title_len = strlen(title);
  int width =
      term_width / 2 >= title_len ? term_width / 2 : title_len - borders;
  int y = term_height / 3 >= 3 ? term_height / 3 : 3;
  int x = (term_width - width) / 2;

  bp->title = title;
  bp->width = width;
  bp->height = POPUP_DEF_HEIGHT;
  bp->x = x;
  bp->y = y;
}

void popup_scale(BaseWin *bw) {
  BaseParam bp = {0};
  popup_gen_def_param(bw->title, &bp);
  bw->win = create_window(&bp);
}
