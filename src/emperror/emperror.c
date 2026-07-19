#include "emperror.h"
#include <asm-generic/ioctls.h>
#include <ncurses.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

int term_width, term_height;

static Panel *PANELS[2];
static int activePanel = 0;

static Panel *get_second_panel() {
  int second_index =
      activePanel == PANEL_COUNT - 1 ? activePanel - 1 : activePanel + 1;
  return PANELS[second_index];
}

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

  init_pair(CP_DIR, COLOR_WHITE, -1);
  init_pair(CP_EXE_SCR, COLOR_GREEN, -1);
  init_pair(CP_EXE_BIN, COLOR_RED, -1);
  init_pair(CP_SELECTED_ITEM, COLOR_BLUE, COLOR_WHITE);
  init_pair(CP_BIN_DATA, COLOR_BLUE, -1);
  init_pair(CP_BASE, COLOR_WHITE, -1);

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
  left_panel.history = (NavHistory *)malloc(sizeof(NavHistory));

  right_panel.win = right_win;
  right_panel.selected_item = 0;
  right_panel.history = (NavHistory *)malloc(sizeof(NavHistory));

  PANELS[0] = &left_panel;
  PANELS[1] = &right_panel;

  getcwd(left_panel.path, sizeof(left_panel.path));
  getcwd(right_panel.path, sizeof(right_panel.path));

  scale_interface();
  reload_panel(&left_panel, true);
  reload_panel(&right_panel, false);

  doupdate();

  int ch;

  while ((ch = getch()) != 'q') {
    Panel *panel = PANELS[activePanel];

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

    } else if (ch == 'a') {
      char title[] = "Enter name";
      int title_len = strlen(title);
      int borders = 2;

      int popup_width =
          term_width / 2 >= title_len ? term_width / 2 : title_len - borders;
      int popup_y = term_height / 3 >= 3 ? term_height / 3 : 3;
      int popup_x = (term_width - popup_width) / 2;

      WINDOW *popup_win =
          create_popup_win(title, 4, popup_width, popup_y, popup_x);
      // вынеси этот метод в отдельный файл popup_win
      wrefresh(popup_win);

      char input_string[80];
      handle_win_input(popup_win, input_string, 80, false, true);

      int input_len = strlen(input_string);
      if (input_len <= 0) {
        continue;
      }

      char path[82];
      append_path_segment(panel->path, input_string, path, 82);

      int status = 0;
      if (path[strlen(path) - 1] == '/' && (status = mkdir(path, 0755)) < 0) {
        mvwprintw(popup_win, 2, 1, "%s", "Can't create dir");
      } else if ((status = open(path, O_CREAT | O_EXCL | O_WRONLY, 0644)) < 0) {
        mvwprintw(popup_win, 2, 1, "%s", "Can't create file");
        close(status);
      }

      if (!status) {
        wait_input(popup_win);
      }

      reload_panel(panel, true);
      Panel *second_panel = get_second_panel();
      reload_panel(second_panel, false);
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
      PANELS[activePanel]->active = true;

      switch_panel(panel, PANELS[activePanel]);
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
    } else if (ch == 'u') {
      char *path = history_go_back(panel->history);
      if (path != NULL) {
        strcpy(panel->path, path);
        enter_dir(panel);
      }
    } else if (ch == 'i') {
      char *path = history_go_forward(panel->history);
      if (path != NULL) {
        strcpy(panel->path, path);
        enter_dir(panel);
      }
    } else if (ch == 'd') {
      char new_path[PATH_MAX];
      FileInfo *fileInfo = &panel->items[panel->selected_item];
      char *name = fileInfo->name;
      append_path_segment(panel->path, name, new_path, PATH_MAX);

      char title[] = "Are you sure?";
      int title_len = strlen(title);
      int borders = 2;
      int popup_width =
          term_width / 2 >= title_len ? term_width / 2 : title_len - borders;
      int popup_y = term_height / 3 >= 3 ? term_height / 3 : 3;
      int popup_x = (term_width - popup_width) / 2;
      WINDOW *alert_win =
          create_alert_dialog(title, 4, popup_width, popup_y, popup_x);

      char input_buf[80];
      handle_win_input(alert_win, input_buf, sizeof(input_buf), true, false);
      if (input_buf[0] == 'Y') {
        remove_dir(new_path);
      }
      reload_panel(panel, true);
      reload_panel(get_second_panel(), false);
    } else if (ch == 10) { // Enter
      enter_file(panel);
    } else if (ch == 263) { // Backspace
      exit_file(panel);
    } else if (ch == 'R') {
      refresh_panel(panel);
    }
    doupdate();
  }

  endwin();
}
