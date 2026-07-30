#include "ui.h"

void erase_area(WINDOW *win, int x1, int x2, int y1, int y2) {
  int max_x = getmaxx(win);

  for (int i = y1; i <= y2; i++) {
    mvwhline(win, i, x1, ' ', x2);
  }
}

void printw_vline(WINDOW *win, int x, int y, int length) {
  for (int i = y; i < length + 1; i++) {
    mvwaddch(win, i, x, ACS_VLINE);
  }
}

void printw_hline(WINDOW *win, int x, int y, int length) {
  for (int i = x; i < length + 1; i++) {
    mvwaddch(win, y, i, ACS_HLINE);
  }
}

void printw_str(WINDOW *win, char *val, int x, int y) {
  mvwprintw(win, y, x, "%s", val);
}

void printw_color_str(WINDOW *win, char *val, int attr, int x, int y) {
  color_on(win, attr);
  printw_str(win, val, x, y);
  color_off(win, attr);
}

void printw_int(WINDOW *win, int val, int x, int y) {
  mvwprintw(win, y, x, "%d", val);
}

void color_on(WINDOW *win, int attr) { wattron(win, attr); }

void color_off(WINDOW *win, int attr) { wattroff(win, attr); }

int get_max_x(WINDOW *win) { return getmaxx(win); }

int get_max_y(WINDOW *win) { return getmaxy(win); }

void refresh_win(WINDOW *win) { wnoutrefresh(win); }
