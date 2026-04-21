#include "panel.h"
#include <stdio.h>
#include <string.h>

static int get_color_pair(FileType file_type) {
  switch (file_type) {
  case FILE_TYPE_DIRECTORY:
    return COLOR_PAIR(CP_DIR) | A_BOLD;
    break;
  case FILE_TYPE_EXECUTABLE_SCRIPT:
    return COLOR_PAIR(CP_EXE_SCR) | A_BOLD;
    break;
  case FILE_TYPE_EXECUTABLE_BINARY:
    return COLOR_PAIR(CP_EXE_BIN);
    break;
  case FILE_TYPE_BINARY_DATA:
  case FILE_TYPE_TEXT_PLAIN:
    return COLOR_PAIR(CP_BIN_DATA);
  }
}

static void truncate_name(char *name, FileType file_type, int max_x) {

  int name_len = strlen(name);
  int name_hborder = NAME_HBORDER(max_x);
  int diff = 0;

  switch (file_type) {
  case FILE_TYPE_DIRECTORY:
  case FILE_TYPE_EXECUTABLE_BINARY:
  case FILE_TYPE_EXECUTABLE_SCRIPT:
    name_len += 2;
    diff -= 2;
    break;
  case FILE_TYPE_TEXT_PLAIN:
  case FILE_TYPE_BINARY_DATA:
    name_len += 1;
    diff += -1;
    break;
  }

  if (name_len >= name_hborder) {
    diff += name_len - (name_len - name_hborder);
    name[diff - 1] = '\0';
  }
}

static void init_header_gaps(int header_len, int word_len, int *left_gap,
                             int *right_gap) {
  if (header_len < word_len) {
    *left_gap = 1;
    *right_gap = 1;
  } else {
    *left_gap = (header_len - word_len) / 2;
    *right_gap = header_len - word_len - *left_gap;
  }
}

static void truncate_size(char *size, int start_x, int end_x) {
  int size_len = strlen(size);

  int size_end = start_x + size_len;

  if (size_end >= end_x) {
    int diff = size_end - end_x - 1;
    size[size_len - diff - 1] = '\0';
  }
}

static void truncate_date(char *date, int start_x, int end_x) {
  int date_len = strlen(date);

  int date_end = start_x + date_len;

  if (date_end >= end_x) {
    int diff = date_end - end_x - 1;
    date[date_len - diff - 1] = '\0';
  }
}

static void display_dir_item_by_type(WINDOW *win, char *name,
                                     FileType file_type, int y,
                                     bool highlight) {

  int color_pair = COLOR_PAIR(CP_SELECTED_ITEM);

  if (!highlight) {
    color_pair = get_color_pair(file_type);
  }

  color_on(win, color_pair);

  int max_x = get_max_x(win);
  char trunc_name[strlen(name)];
  strcpy(trunc_name, name);

  truncate_name(trunc_name, file_type, max_x);

  switch (file_type) {
  case FILE_TYPE_DIRECTORY:
    printw_str(win, trunc_name, 1, y, "/%s");
    break;
  case FILE_TYPE_EXECUTABLE_BINARY:
    printw_str(win, trunc_name, 2, y, "@%s");
    break;
  case FILE_TYPE_EXECUTABLE_SCRIPT:
    printw_str(win, trunc_name, 2, y, "*%s");
    break;
  case FILE_TYPE_TEXT_PLAIN:
  case FILE_TYPE_BINARY_DATA:
    printw_str(win, trunc_name, 2, y, "%s");
    break;
  }

  color_off(win, color_pair);
}

void display_dir_item(Panel *panel, int item_pos, bool highlight) {
  int y = get_y(item_pos, PAGE_SIZE);

  FileInfo *file_info = &panel->items[item_pos];

  display_dir_item_by_type(panel->win, file_info->name, file_info->file_type, y,
                           highlight);
}

void move_selection(Panel *panel, int position) {
  int page = (panel->selected_item / PAGE_SIZE) + 1;
  if (panel == NULL || position > page * PAGE_SIZE - 1 ||
      position < PAGE_SIZE * (page - 1)) {
    return;
  }

  int old_selection = panel->selected_item;
  int new_selection = position;

  display_dir_item(panel, old_selection, false);
  display_dir_item(panel, new_selection, true);

  refresh_win(panel->win);
}

void switch_panel(Panel *old_panel, Panel *new_panel) {
  display_dir_item(old_panel, old_panel->selected_item, false);
  display_dir_item(new_panel, new_panel->selected_item, true);

  refresh_win(old_panel->win);
  refresh_win(new_panel->win);
}

int get_index_dir_by_name(Panel *panel, char *dir_name) {
  for (int i = 0; i < panel->count; i++) {
    FileInfo *file_info = &panel->items[i];
    if (strcmp_(file_info->name, dir_name) == 0) {
      return i;
    }
  }

  return 0;
}

void erase_dir_area(Panel *panel) {
  WINDOW *win = panel->win;
  int max_x = get_max_x(win);

  erase_area(win, 1, NAME_HBORDER(max_x) - 1, Y_TOP_OFFSET, Y_BOTTOM_OFFSET);
  erase_area(win, NAME_HBORDER(max_x) + 1,
             SIZE_HBORDER(max_x) - NAME_HBORDER(max_x) - 1, Y_TOP_OFFSET,
             Y_BOTTOM_OFFSET);
  erase_area(win, SIZE_HBORDER(max_x) + 1,
             DATE_HBORDER(max_x) - SIZE_HBORDER(max_x) - 1, Y_TOP_OFFSET,
             Y_BOTTOM_OFFSET);
}

void display_headers_names(Panel *panel) {
  WINDOW *win = panel->win;
  int max_x = get_max_x(win);
  int name_x = NAME_HBORDER(max_x) / 2 - 1;
  int size_x =
      NAME_HBORDER(max_x) + (SIZE_HBORDER(max_x) - NAME_HBORDER(max_x)) / 2 - 1;
  int date_x =
      SIZE_HBORDER(max_x) + (DATE_HBORDER(max_x) - SIZE_HBORDER(max_x)) / 2 - 2;

  printw_str(win, "Name", name_x, 1, "%s");
  printw_str(win, "Size", size_x, 1, "%s");
  printw_str(win, "Data", date_x, 1, "%s");
}

void display_headers_hborders(Panel *panel) {
  WINDOW *win = panel->win;
  int max_x = get_max_x(win);

  int gap1 = max_x - DATE_COL_WIDTH - SIZE_COL_WIDTH;
  int gap2 = max_x - DATE_COL_WIDTH;
  int gap3 = max_x - 1;

  printw_hline(win, 1, 2, gap1);
  printw_hline(win, gap1 + 1, 2, gap2);
  printw_hline(win, gap2 + 1, 2, gap3);
}

void display_headers_vborders(Panel *panel) {
  WINDOW *win = panel->win;
  int max_x = get_max_x(win);

  printw_vline(win, NAME_HBORDER(max_x), 1, Y_BOTTOM_OFFSET);
  printw_vline(win, SIZE_HBORDER(max_x), 1, Y_BOTTOM_OFFSET);
  printw_vline(win, DATE_HBORDER(max_x), 1, Y_BOTTOM_OFFSET);
}

void display_ui(Panel *panel) {
  display_headers_names(panel);
  display_headers_hborders(panel);
  display_headers_vborders(panel);

  refresh_win(panel->win);
}

void display_dir(Panel *panel) {

  WINDOW *win = panel->win;
  FileInfo *items = panel->items;

  int start = panel->selected_item;
  int end = panel->count;

  erase_dir_area(panel);

  struct tm tm;
  char datebuf[64];
  char sizebuf[12];

  int max_x = get_max_x(win);
  int max_y = get_max_y(win);

  int y = Y_TOP_OFFSET;

  int color_pair = get_color_pair(CP_DIR);

  for (int i = start; i < end; i++) {

    FileInfo *item = items + i;

    if (y >= Y_BOTTOM_OFFSET) {
      break;
    }

    display_dir_item_by_type(win, item->name, item->file_type, y, false);

    int size = item->size;

    time_t time = item->mod_time;
    localtime_r(&time, &tm);
    strftime(datebuf, sizeof(datebuf), "%Y-%m-%d %H:%M:%S", &tm);
    snprintf(sizebuf, sizeof(sizebuf), "%d", size);

    int size_header_len = SIZE_HBORDER(max_x) - NAME_HBORDER(max_x);
    int date_header_len = DATE_HBORDER(max_x) - SIZE_HBORDER(max_x);

    int date_len = strlen(datebuf);
    int size_len = strlen(sizebuf);

    int size_left_gap = 0;
    int size_right_gap = 0;
    init_header_gaps(size_header_len, size_len, &size_left_gap,
                     &size_right_gap);

    int date_left_gap = 0;
    int date_right_gap = 0;
    init_header_gaps(date_header_len, date_len, &date_left_gap,
                     &date_right_gap);

    int size_start_x = NAME_HBORDER(max_x) + size_left_gap;
    int size_end_x = SIZE_HBORDER(max_x) - size_right_gap;

    int date_start_x = SIZE_HBORDER(max_x) + date_left_gap;
    int date_end_x = DATE_HBORDER(max_x) - date_right_gap;

    truncate_size(sizebuf, size_start_x, size_end_x);
    truncate_date(datebuf, date_start_x, date_end_x);

    printw_str(win, sizebuf, size_start_x + 1, y, "%s");
    printw_str(win, datebuf, date_start_x + 1, y, "%s");

    y++;
  }

  refresh_win(win);
}

void exit_dir(Panel *panel) {

  char entry_name[128];
  get_last_segment(panel->path, entry_name, sizeof(entry_name));

  substract_path_segment(panel->path, panel->path, 512);
  panel->selected_item = 0;

  int elements = load_dir(panel->path, panel->items, 512);
  panel->count = elements;

  sort_dir(panel->items, panel->count);
  display_dir(panel);

  panel->selected_item = get_index_dir_by_name(panel, entry_name);
  move_selection(panel, panel->selected_item);
}

void enter_dir(Panel *panel) {
  FileInfo *fileInfo = &panel->items[panel->selected_item];
  char *name = fileInfo->name;
  int type = fileInfo->file_type;

  if (type == FILE_TYPE_DIRECTORY && panel->count > 0) {

    if (strcmp_(name, "..") == 0) {
      exit_dir(panel);
      return;
    }

    char result[2048]; // TO DO: check append_path_segment
    strcpy(result, panel->path);
    append_path_segment(panel->path, fileInfo->name, result, 2048);
    strcpy(panel->path, result);

    panel->selected_item = 0;

    int elements = load_dir(panel->path, panel->items, 512);
    panel->count = elements;

    sort_dir(panel->items, panel->count);
    display_dir(panel);

    display_dir(panel);
    move_selection(panel, panel->selected_item);

  } else {
    def_prog_mode();
    endwin();

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "vim %s/%s", panel->path, fileInfo->name);
    int result = system(cmd);

    reset_prog_mode();
    refresh_win(panel->win);
  }
}
