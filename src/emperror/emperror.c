#include "emperror.h"
#include <asm-generic/ioctls.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

int term_width, term_height;

static Panel *PANELS[2];
static int activePanel = 0;
static Promt *promt;
static Dialog *dialog;

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
  left_panel.history = (History *)malloc(sizeof(History));

  right_panel.win = right_win;
  right_panel.selected_item = 0;
  right_panel.history = (History *)malloc(sizeof(History));

  PANELS[0] = &left_panel;
  PANELS[1] = &right_panel;

  BaseParam promt_bp = {0};
  popup_gen_def_param("Enter name", &promt_bp);
  promt = popup_create_promt(&promt_bp);

  BaseParam dialog_bp = {0};
  popup_gen_def_param("Are you sure?", &dialog_bp);

  DialogParam dp = {0};
  dp.accept = "(Y)es";
  dp.decline = "(N)o";
  dialog = popup_create_dialog(&dialog_bp, &dp);

  getcwd(left_panel.path, sizeof(left_panel.path));
  getcwd(right_panel.path, sizeof(right_panel.path));

  panel_scale_interface();
  panel_reload(&left_panel, true);
  panel_reload(&right_panel, false);

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

      panel_scale_interface();
      popup_scale(&promt->base_win);
      popup_scale(&dialog->base_win);

      box(left_panel.win, 0, 0);
      box(right_panel.win, 0, 0);

      panel_draw(&left_panel);
      panel_draw(&right_panel);
      panel_toggle_highlight(panel, true);

      wnoutrefresh(left_panel.win);
      wnoutrefresh(right_panel.win);

    } else if (ch == 'a') {
      WINDOW *promt_win = promt->base_win.win;
      touchwin(promt_win);
      wrefresh(promt_win);

      char input_string[80];
      promt->base_win.handle_input(promt_win, input_string, 80);

      int input_len = strlen(input_string);
      if (input_len <= 0) {
        goto reload;
        continue;
      }

      char path[82];
      append_path_segment(panel->path, input_string, path, 82);

      int status = 0;
      if (path[strlen(path) - 1] == '/') {
        status = mkdir(path, 0755);
      } else {
        status = open(path, O_CREAT | O_EXCL | O_WRONLY, 0644);
      }

      if (status < 0) {
        mvwprintw(promt_win, 2, 1, "%s", "Can't create file entity");
        promt->base_win.wait_input(promt_win);
      }

    reload:
      panel_reload(panel, true);
      panel_reload(get_second_panel(), false);

    } else if (ch == 'k' && panel->selected_item > 0) { // w //119
      int new_pos = 0;
      if (get_y(panel->selected_item, PAGE_SIZE) == Y_TOP_OFFSET) {
        panel->selected_item = panel->selected_item - PAGE_SIZE;
        panel_draw_dir(panel);
        new_pos = panel->selected_item + (PAGE_SIZE - 1) / 2;
        panel_move_selection(panel, new_pos);
      } else {
        new_pos = panel->selected_item - 1;
        panel_move_selection(panel, new_pos);
      }
    } else if (ch == 'j' &&
               panel->selected_item < panel->count - 1) { // s //115
      if (get_y(panel->selected_item, PAGE_SIZE) == Y_BOTTOM_OFFSET) {
        panel->selected_item++;

        int new_selected_item = 0;
        int diff = panel->count - (panel->selected_item + 1);

        panel_draw_dir(panel);

        if (diff < PAGE_SIZE - 1) {
          new_selected_item = panel->selected_item + diff / 2;
        } else {
          new_selected_item = panel->selected_item + (PAGE_SIZE - 1) / 2;
        }
        panel_move_selection(panel, new_selected_item);
      } else {
        panel_move_selection(panel, panel->selected_item + 1);
      }
    } else if (ch == 9) { // Tab
      if (activePanel == PANEL_COUNT - 1)
        activePanel = 0;
      else
        activePanel++;

      panel->active = false;
      PANELS[activePanel]->active = true;

      panel_switch(panel, PANELS[activePanel]);
    } else if (ch == 'K') { // page up
      int selected_item = panel->selected_item;

      int current_page = selected_item / PAGE_SIZE;
      int first_page = 0;

      if (current_page == first_page) {
        continue;
      }

      int new_selected_item = selected_item - PAGE_SIZE;
      panel->selected_item = new_selected_item;

      panel_draw_dir(panel);
      panel_toggle_highlight(panel, true);
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
      panel_draw_dir(panel);
      panel_toggle_highlight(panel, true);
    } else if (ch == 'u') {
      char *path = history_go_back(panel->history);
      if (path != NULL) {
        strcpy(panel->path, path);
        panel_enter_dir(panel);
      }
    } else if (ch == 'i') {
      char *path = history_go_forward(panel->history);
      if (path != NULL) {
        strcpy(panel->path, path);
        panel_enter_dir(panel);
      }
    } else if (ch == 'd') {
      char new_path[PATH_MAX];
      FileInfo *fileInfo = &panel->items[panel->selected_item];
      char *name = fileInfo->name;
      append_path_segment(panel->path, name, new_path, PATH_MAX);

      WINDOW *dialog_win = dialog->base_win.win;
      touchwin(dialog_win);
      wrefresh(dialog_win);

      char input_buf[2];
      dialog->base_win.handle_input(dialog_win, input_buf, 2);
      if (input_buf[0] == 'Y') {
        remove_dir(new_path);
      }
      panel_reload(panel, true);
      panel_reload(get_second_panel(), false);
    } else if (ch == 10) { // Enter
      panel_enter_file(panel);
    } else if (ch == 263) { // Backspace
      panel_exit_file(panel);
    } else if (ch == 'R') {
      panel_refresh(panel);
    }
    doupdate();
  }

  endwin();
}
