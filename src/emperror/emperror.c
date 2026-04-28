#include "emperror.h"
#include <asm-generic/ioctls.h>
#include <ncurses.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

void commander_run() {
  initscr();

  if (has_colors() == false) {
    endwin();
    printw("Your terminal does not support colors");
    exit(1);
  }

  cbreak();
  use_env(FALSE);
  noecho();
  keypad(stdscr, TRUE);
  start_color();
  use_default_colors();
  curs_set(0);

  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_RED, COLOR_BLACK);
  init_pair(4, COLOR_BLUE, COLOR_WHITE);

  clear();
  refresh();

  int height, width;
  getmaxyx(stdscr, height, width);
  WINDOW *left_win = newwin(LINES, COLS / 2, 0, 0);
  WINDOW *right_win = newwin(LINES, COLS / 2, 0, COLS / 2);

  Panel right_panel = {0};
  Panel left_panel = {0};

  left_panel.win = left_win;
  left_panel.active = true;
  left_panel.selected_item = 0;

  right_panel.win = right_win;
  right_panel.selected_item = 0;

  Panel *panels[] = {&left_panel, &right_panel};

  getcwd(left_panel.path, sizeof(left_panel.path));
  getcwd(right_panel.path, sizeof(right_panel.path));

  //  strcpy(right_panel.path, "/bin");
  // strcpy(left_panel.path, "/bin");

  int quantity = load_dir(left_panel.path, left_panel.items, 512);
  left_panel.count = quantity;

  quantity = load_dir(right_panel.path, right_panel.items, 512);
  right_panel.count = quantity;

  sort_dir(left_panel.items, left_panel.count);
  sort_dir(right_panel.items, right_panel.count);

  box(left_win, 0, 0);
  box(right_win, 0, 0);
  draw_ui(&left_panel);
  draw_ui(&right_panel);
  draw_dir(&left_panel);
  move_selection(&left_panel, left_panel.selected_item);
  draw_dir(&right_panel);
  doupdate();

  int ch;
  int activePanel = 0;

  while ((ch = getch()) != 'q') {
    Panel *panel = panels[activePanel];

    if (ch == KEY_RESIZE) {
      endwin();
      initscr();

      cbreak();
      noecho();
      keypad(stdscr, TRUE);
      start_color();
      use_default_colors();
      curs_set(0);

      clear();
      refresh();

      left_panel.win = newwin(LINES, COLS / 2, 0, 0);
      right_panel.win = newwin(LINES, COLS / 2, 0, COLS / 2);

      box(left_panel.win, 0, 0);
      box(right_panel.win, 0, 0);

      draw_ui(&left_panel);
      draw_ui(&right_panel);
      draw_dir(&left_panel);
      draw_dir(&right_panel);
      move_selection(panel, panel->selected_item);

      wnoutrefresh(left_panel.win);
      wnoutrefresh(right_panel.win);
      doupdate();

    } else if (ch == 'k' && panel->selected_item > 0) { // w //119
      if (get_y(panel->selected_item, PAGE_SIZE) == Y_TOP_OFFSET) {
        panel->selected_item = panel->selected_item - PAGE_SIZE;
        draw_dir(panel);
        panel->selected_item = panel->selected_item + (PAGE_SIZE - 1) / 2;
        move_selection(panel, panel->selected_item);
      } else {
        move_selection(panel, panel->selected_item - 1);
        panel->selected_item--;
      }
    } else if (ch == 'j' &&
               panel->selected_item < panel->count - 1) { // s //115
      if (get_y(panel->selected_item, PAGE_SIZE) == Y_BOTTOM_OFFSET) {
        panel->selected_item++;

        int new_selected_item = 0;
        int diff = panel->count - (panel->selected_item + 1);

        draw_dir(panel);

        if (diff < PAGE_SIZE - 1) {
          new_selected_item = panel->selected_item + diff / 2;
        } else {
          new_selected_item = panel->selected_item + (PAGE_SIZE - 1) / 2;
        }

        panel->selected_item = new_selected_item;
        move_selection(panel, panel->selected_item);
      } else {
        move_selection(panel, panel->selected_item + 1);
        panel->selected_item++;
      }
    } else if (ch == 9) { // Tab
      if (activePanel == PANEL_COUNT - 1)
        activePanel = 0;
      else
        activePanel++;

      panel->active = false;
      panels[activePanel]->active = true;

      switch_panel(panel, panels[activePanel]);
    } else if (ch == 10) { // Enter

      enter_dir(panel);
    } else if (ch == 263) { // Backspace
      exit_dir(panel);
    }

    doupdate();
  }

  endwin();
}
