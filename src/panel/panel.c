#include "panel.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

extern int term_width, term_height;

static ColumnDef cols[] = {
    {"Name", 15, 8, 0, 50, 2},
    {"Size", 6, 4, 0, 8, 1},
    {"Modify time", 12, 5, 0, 16, 1},
};

static int cols_size = sizeof(cols) / sizeof(cols[0]);

void scale_interface() {
  int padding = 2;
  int dividing_lines = 2;

  int columns_space = 0;
  for (int i = 0; i < cols_size; i++) {
    columns_space += cols[i].desired_width;
    cols[i].width = cols[i].desired_width;
  }

  int ocuppied_space = padding + dividing_lines + columns_space;
  int available_space = term_width / 2 - ocuppied_space;

  if (available_space == 0) {
    return;
  } else if (available_space > 0) {
    cols[NAME].width = cols[NAME].desired_width + available_space;

  } else if (available_space < 0) {
    int lack = abs(available_space);

    int name_diff = cols[NAME].desired_width - cols[NAME].min_width;

    if (lack - name_diff <= 0) {
      cols[NAME].width = cols[NAME].min_width + abs(lack - name_diff);
      return;
    }

    bool scaled = false;
    for (int i = 0; i < cols_size; i++) {
      ColumnDef *col = &cols[i];
      int new_width = col->desired_width;

      while (lack > 0 && new_width > col->min_width) {
        new_width--;
        lack--;
      }

      col->width = new_width;

      if (lack <= 0) {
        scaled = true;
        break;
      }
    }

    if (!scaled) {
      for (int i = 1; i < cols_size; i++) {
        ColumnDef *col = &cols[i];

        int width = col->width;
        col->width = 0;

        lack -= (width + 1);

        if (lack <= 0) {
          cols[NAME].width += abs(lack);
          scaled = true;
          break;
        }
      }
    }

    if (!scaled) {
      cols[NAME].width = cols[NAME].width - lack;
    }
  }
}

static int get_vborder_x(ColumnDef *col, ColumnDef *cols) {
  int i = 0;
  int vborder_x = col->width + 1;

  while (strcmp(col->header, cols[i].header) != 0) {
    vborder_x += (cols[i].width + 1);
    i++;
  }

  return vborder_x;
}

static int get_header_x(ColumnDef *col, ColumnDef *cols) {
  int header_x = 0;
  int vborder_x = get_vborder_x(col, cols);

  if (strcmp(col->header, "Name") == 0) {
    header_x = (vborder_x - 1) / 2 - 1;
  } else {
    int start_zone_x = vborder_x - col->width;
    int left_gap = col->width - strlen(col->header);
    header_x = start_zone_x + left_gap;
  }

  return header_x;
}

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
  int name_hborder = get_vborder_x(&cols[NAME], cols);
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

static void draw_entry_name_internal(WINDOW *win, char *name,
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

static void draw_entry_name(Panel *panel, int item_pos, bool highlight) {
  int y = get_y(item_pos, PAGE_SIZE);

  FileInfo *file_info = &panel->items[item_pos];

  draw_entry_name_internal(panel->win, file_info->name, file_info->file_type, y,
                           highlight);
}

static void draw_entry_size(WINDOW *win, char *sizebuf, int x, int y) {
  int size_header_len = cols[SIZE].width;
  int size_len = strlen(sizebuf);

  int size_left_gap = 0;

  int diff = abs(size_header_len - size_len);
  if (size_len <= size_header_len) {
    size_left_gap = diff;
  } else if (diff <= 2) {
    sizebuf[size_len - 3] = 'K';
    sizebuf[size_len - 2] = '\0';
    size_left_gap = size_header_len - (size_len - 2);
  } else if (diff >= 3 && diff <= 8) {
    sizebuf[size_len - 6] = 'M';
    sizebuf[size_len - 5] = '\0';
    size_left_gap = size_header_len - (size_len - 5);
  } else if (diff >= 9 && diff <= 11) {
    sizebuf[size_len - 9] = 'G';
    sizebuf[size_len - 8] = '\0';
    size_left_gap = size_header_len - (size_len - 8);
  }

  printw_str(win, sizebuf, x + size_left_gap, y, "%s");
}

static void draw_entry_mod_date(WINDOW *win, char *datebuf, int x, int y) {
  int date_header_len = cols[MOD_TIME].width;
  int date_len = strlen(datebuf);

  int diff = date_header_len - date_len;

  if (diff < 0) {

    int new_date_len = 0;
    int offset = 0;

    if (diff >= -3) {
      new_date_len = date_len - 3;
      offset = 3;
    } else {
      diff = abs(diff);
      new_date_len = date_len - diff;
      offset = diff;
    }

    char truncate_date[new_date_len];
    memcpy(datebuf, &datebuf[offset], sizeof(char) * new_date_len);
    datebuf[new_date_len] = '\0';
  }

  printw_str(win, datebuf, x, y, "%s");
}

void move_selection(Panel *panel, int position) {
  int page = (panel->selected_item / PAGE_SIZE) + 1;
  if (panel == NULL || position > page * PAGE_SIZE - 1 ||
      position < PAGE_SIZE * (page - 1)) {
    return;
  }

  int old_selection = panel->selected_item;
  int new_selection = position;

  draw_entry_name(panel, old_selection, false);
  draw_entry_name(panel, new_selection, true);

  refresh_win(panel->win);
}

void switch_panel(Panel *old_panel, Panel *new_panel) {
  draw_entry_name(old_panel, old_panel->selected_item, false);
  draw_entry_name(new_panel, new_panel->selected_item, true);

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

  int name_vborder = get_vborder_x(&cols[NAME], cols);
  int size_vborder = get_vborder_x(&cols[SIZE], cols);
  int mod_time_vborder = get_vborder_x(&cols[MOD_TIME], cols);

  erase_area(win, 1, name_vborder - 1, Y_TOP_OFFSET, Y_BOTTOM_OFFSET);
  erase_area(win, name_vborder + 1, size_vborder - name_vborder - 1,
             Y_TOP_OFFSET, Y_BOTTOM_OFFSET);
  erase_area(win, size_vborder + 1, mod_time_vborder - size_vborder - 1,
             Y_TOP_OFFSET, Y_BOTTOM_OFFSET);
}

void draw_headers_names(Panel *panel) {
  WINDOW *win = panel->win;
  for (int i = 0; i < cols_size; i++) {
    ColumnDef *col = &cols[i];
    if (col->width != 0) {
      printw_str(win, col->header, get_header_x(col, cols), 1, "%s");
    }
  }
}

void draw_headers_hborders(Panel *panel) {
  WINDOW *win = panel->win;
  int x = 1;
  int length = 0;

  for (int i = 0; i < cols_size; i++) {
    ColumnDef *col = &cols[i];

    if (col->width != 0) {
      int vborder_x = get_vborder_x(col, cols);
      printw_hline(win, x, 2, vborder_x);
      length += vborder_x;
    }
  }
}

void draw_headers_vborders(Panel *panel) {
  WINDOW *win = panel->win;

  for (int i = 0; i < cols_size; i++) {
    ColumnDef *col = &cols[i];

    if (col->width != 0) {
      printw_vline(win, get_vborder_x(col, cols), 1, Y_BOTTOM_OFFSET);
    }
  }
}

void draw_ui(Panel *panel) {
  draw_headers_names(panel);
  draw_headers_hborders(panel);
  draw_headers_vborders(panel);

  refresh_win(panel->win);
}

void draw_dir(Panel *panel) {

  WINDOW *win = panel->win;
  FileInfo *items = panel->items;

  int item_y = get_y(panel->selected_item, PAGE_SIZE);
  int start = panel->selected_item - (item_y - Y_TOP_OFFSET);
  int end = panel->selected_item + (Y_BOTTOM_OFFSET - item_y);

  erase_dir_area(panel);

  struct tm tm;
  time_t now;
  struct tm local_time;
  char *time_format;

  char datebuf[64];
  char sizebuf[12];

  time(&now);

  int max_x = get_max_x(win);
  int max_y = get_max_y(win);

  int y = Y_TOP_OFFSET;
  int color_pair = get_color_pair(CP_DIR);

  for (int i = start; i <= end && i < panel->count; i++) {
    FileInfo *item = items + i;

    int size = item->size;
    snprintf(sizebuf, sizeof(sizebuf), "%d", size);

    time_t item_time = item->mod_time;
    localtime_r(&item_time, &tm);
    localtime_r(&now, &local_time);

    if (tm.tm_year < local_time.tm_year) {
      time_format = "%d %b %Y";
    } else {
      time_format = "%b %d %H:%M";
    }

    strftime(datebuf, sizeof(datebuf), time_format, &tm);

    draw_entry_name_internal(win, item->name, item->file_type, y, false);

    if (cols[SIZE].width > 0) {
      int size_start_x = get_vborder_x(&cols[NAME], cols) + 1;
      draw_entry_size(win, sizebuf, size_start_x, y);
    }

    if (cols[MOD_TIME].width > 0) {
      int date_start_x = get_vborder_x(&cols[SIZE], cols) + 1;
      draw_entry_mod_date(win, datebuf, date_start_x, y);
    }

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
  draw_dir(panel);

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
    draw_dir(panel);

    draw_dir(panel);
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
