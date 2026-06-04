#include "emperror.h"
#include <asm-generic/ioctls.h>
#include <ncurses.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

int term_width, term_height;
bool iLoveColors = false;

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

  init_pair(CP_DIR, COLOR_WHITE, COLOR_BLACK);
  init_pair(CP_EXE_SCR, COLOR_GREEN, COLOR_BLACK);
  init_pair(CP_EXE_BIN, COLOR_RED, COLOR_BLACK);
  init_pair(CP_SELECTED_ITEM, COLOR_BLUE, COLOR_WHITE);
  init_pair(CP_BIN_DATA, COLOR_BLUE, COLOR_BLACK);
  init_pair(CP_BASE, COLOR_WHITE, COLOR_BLACK);

  init_color(COLOR_MAGENTA, 1000, 500, 0);
  init_pair(8, COLOR_MAGENTA, -1);

  clear();
  refresh();

  struct winsize w;

  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &w) == 0) {
    term_width = w.ws_col;
    term_height = w.ws_row; // if error just exit
  } else {
    term_width = COLS;
    term_height = LINES;
  }

  WINDOW *left_win = newwin(term_height, term_width / 2, 0, 0);
  WINDOW *right_win = newwin(term_height, term_width / 2, 0, term_width / 2);

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

  load_sorted_dir(&left_panel);
  load_sorted_dir(&right_panel);

  box(left_win, 0, 0);
  box(right_win, 0, 0);
  scale_interface();
  draw_panel(&left_panel);
  draw_panel(&right_panel);
  toggle_highlight(&left_panel, true);
  wrefresh(left_panel.win);
  wrefresh(right_panel.win);

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

      if (ioctl(STDIN_FILENO, TIOCGWINSZ, &w) == 0) {
        term_width = w.ws_col;
        term_height = w.ws_row;
      }

      left_panel.win = newwin(term_height, term_width / 2, 0, 0);
      right_panel.win = newwin(term_height, term_width / 2, 0, term_width / 2);

      scale_interface();

      box(left_panel.win, 0, 0);
      box(right_panel.win, 0, 0);

      draw_panel(&left_panel);
      draw_panel(&right_panel);
      toggle_highlight(panel, true);

      wnoutrefresh(left_panel.win);
      wnoutrefresh(right_panel.win);

    } else if (ch == 'd') {
      WINDOW *popup_win = create_popup_win("Enter dir name");
      wrefresh(popup_win);

      int popup_width;
      getmaxx(popup_win);

      char input_string[80];
      handle_win_input(popup_win, input_string, 80);

      char result[82];
      append_path_segment(panel->path, input_string, result, 82);

      int status = mkdir(result, 0755);

      if (!status) {
        mvwprintw(popup_win, 2, 1, "%s", "Can't create dir");
      }

      load_sorted_dir(panel);
      draw_panel(panel);
      toggle_highlight(panel, true);
      box(panel->win, 0, 0);
      wrefresh(panel->win);

      int second_index =
          activePanel == PANEL_COUNT - 1 ? activePanel - 1 : activePanel + 1;
      Panel *second_panel = panels[second_index];
      load_sorted_dir(second_panel);
      draw_panel(second_panel);
      box(second_panel->win, 0, 0);
      wrefresh(second_panel->win);

      delwin(popup_win);
      ch = 0;
    } else if (ch == 'f') {
      WINDOW *popup_win = create_popup_win("Enter file name");
      wrefresh(popup_win);

      int popup_width;
      getmaxx(popup_win);

      char input_string[80];
      handle_win_input(popup_win, input_string, 80);

      char path[82];
      append_path_segment(panel->path, input_string, path, 82);

      int fd = open(path, O_CREAT | O_EXCL | O_WRONLY, 0644);

      if (fd == -1) {
        mvwprintw(popup_win, 2, 1, "%s", "Can't create file");
      }

      close(fd);

      load_sorted_dir(panel);
      draw_panel(panel);
      toggle_highlight(panel, true);
      box(panel->win, 0, 0);
      wrefresh(panel->win);

      int second_index =
          activePanel == PANEL_COUNT - 1 ? activePanel - 1 : activePanel + 1;
      Panel *second_panel = panels[second_index];
      load_sorted_dir(second_panel);
      draw_panel(second_panel);
      box(second_panel->win, 0, 0);
      wrefresh(second_panel->win);

      delwin(popup_win);
    } else if (ch == 'k' && panel->selected_item > 0) { // w //119
      int new_pos = 0;
      if (get_y(panel->selected_item, PAGE_SIZE) == Y_TOP_OFFSET) {
        panel->selected_item = panel->selected_item - PAGE_SIZE;
        draw_dir(panel);
        new_pos = panel->selected_item + (PAGE_SIZE - 1) / 2;
        move_selection(panel, new_pos);
      } else {
        new_pos = panel->selected_item - 1;
        move_selection(panel, new_pos);
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
        move_selection(panel, new_selected_item);
      } else {
        move_selection(panel, panel->selected_item + 1);
      }
    } else if (ch == 9) { // Tab
      if (activePanel == PANEL_COUNT - 1)
        activePanel = 0;
      else
        activePanel++;

      panel->active = false;
      panels[activePanel]->active = true;

      switch_panel(panel, panels[activePanel]);
    } else if (ch == 'K') { // page up
      int selected_item = panel->selected_item;

      int current_page = selected_item / PAGE_SIZE;
      int first_page = 0;

      if (current_page == first_page) {
        continue;
      }

      int new_selected_item = selected_item - PAGE_SIZE;
      panel->selected_item = new_selected_item;

      draw_dir(panel);
      toggle_highlight(panel, true);
    } else if (ch == 'J') { // page down
      int selected_item = panel->selected_item;

      int current_page = selected_item / PAGE_SIZE;
      int last_page = (panel->count - 1) / PAGE_SIZE;

      if (current_page == last_page) {
        continue;
      }

      int new_selected_item = selected_item + PAGE_SIZE;
      if (new_selected_item > panel->count - 1) {
        int current_entry_y = get_y(selected_item, PAGE_SIZE);
        int last_entry_y = get_y(panel->count - 1, PAGE_SIZE);

        new_selected_item = selected_item +
                            (PAGE_SIZE - (current_entry_y - Y_TOP_OFFSET + 1)) +
                            ((last_entry_y - Y_TOP_OFFSET + 1) / 2);
      }

      panel->selected_item = new_selected_item;
      draw_dir(panel);
      toggle_highlight(panel, true);
    } else if (ch == 10) { // Enter
      enter_dir(panel);
    } else if (ch == 263) { // Backspace
      exit_dir(panel);
    }

    doupdate();
  }

  endwin();
}
