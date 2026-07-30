#include "panel.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

extern int term_width, term_height;

static void draw_name(WINDOW *win, PanelEntry *entry, int x, int y);
static void draw_size(WINDOW *win, PanelEntry *entry, int x, int y);
static void draw_mod_time(WINDOW *win, PanelEntry *entry, int x, int y);

static ColumnDef cols[] = {
    {"Name", 15, 8, 0, 50, 2, draw_name},
    {"Size", 6, 4, 0, 8, 1, draw_size},
    {"Modify time", 12, 5, 0, 16, 1, draw_mod_time},
};

static int cols_size = sizeof(cols) / sizeof(cols[0]);

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

int get_col_draw_x(ColumnDef *col, ColumnDef *cols) {
  int i = 0;
  int x = 1;
  while (strcmp(col->header, cols[i].header) != 0) {
    x += (cols[i].width + 1);
    i++;
  }

  return x;
}

static int append_name_prefix(char *name, FileType file_type,
                              char *formatted_name, int buf_size) {
  if (!name || !formatted_name || buf_size == 0) {
    return -1;
  }

  int name_len = strlen(name);

  if (buf_size - name_len < 2) {
    return -1;
  }

  int insert_index = 0;

  char ch = '/';

  switch (file_type) {
  case FILE_TYPE_DIRECTORY:
    ch = '/';
    break;
  case FILE_TYPE_LNK_TO_DIR:
    ch = '~';
    break;
  case FILE_TYPE_LNK_OTHER:
    ch = '@';
    break;
  case FILE_TYPE_EXECUTABLE_SCRIPT:
  case FILE_TYPE_EXECUTABLE_BINARY:
    ch = '*';
    break;
  case FILE_TYPE_BINARY_DATA:
  case FILE_TYPE_LNK_UNKNOWN:
  case FILE_TYPE_LNK_BROKEN:
  case FILE_TYPE_UNKNOWN:
    ch = ' ';
    break;
  default:
    ch = ch;
    break;
  }

  formatted_name[0] = ch;
  strcpy(formatted_name + 1, name);

  return 1;
}

static void truncate_name(char *name, int x) {
  int name_len = strlen(name);
  int name_vborder = get_vborder_x(&cols[NAME], cols);
  int name_end = x + name_len;

  if (name_end >= name_vborder) {
    int diff = name_end - (name_end - (name_vborder - 1));
    name[diff] = '\0';
  }
}

static int get_color_pair(FileType file_type) {
  switch (file_type) {
  case FILE_TYPE_DIRECTORY:
  case FILE_TYPE_LNK_TO_DIR:
    return COLOR_PAIR(CP_DIR) | A_BOLD;
    break;
  case FILE_TYPE_EXECUTABLE_SCRIPT:
    return COLOR_PAIR(CP_EXE_SCR);
    break;
  case FILE_TYPE_EXECUTABLE_BINARY:
    return COLOR_PAIR(CP_EXE_BIN);
    break;
  case FILE_TYPE_BINARY_DATA:
    return COLOR_PAIR(CP_BIN_DATA);
    break;
  case FILE_TYPE_LNK_OTHER:
  case FILE_TYPE_UNKNOWN:
  default:
    return COLOR_PAIR(CP_BASE);
  }
}

static void draw_name(WINDOW *win, PanelEntry *entry, int x, int y) {
  char *name = entry->name;
  int name_len = strlen(name);
  char formatted_name[name_len + 2];

  int status =
      append_name_prefix(name, entry->file_type, formatted_name, name_len + 2);

  if (!status) {
    strcpy(formatted_name, name);
  }

  truncate_name(formatted_name, x);

  int color_attr = 0;
  if (entry->is_selected) {
    color_attr = COLOR_PAIR(CP_SELECTED_ITEM);
  } else {
    color_attr = get_color_pair(entry->file_type);
  }

  printw_color_str(win, formatted_name, color_attr, x, y);
}

static void draw_size(WINDOW *win, PanelEntry *entry, int x, int y) {
  char *size = entry->size;
  int size_header_len = cols[SIZE].width;
  int size_len = strlen(size);

  int size_left_gap = 0;

  int diff = abs(size_header_len - size_len);
  if (size_len <= size_header_len) {
    size_left_gap = diff;
  } else if (diff <= 2) {
    size[size_len - 3] = 'K';
    size[size_len - 2] = '\0';
    size_left_gap = size_header_len - (size_len - 2);
  } else if (diff >= 3 && diff <= 8) {
    size[size_len - 6] = 'M';
    size[size_len - 5] = '\0';
    size_left_gap = size_header_len - (size_len - 5);
  } else if (diff >= 9 && diff <= 11) {
    size[size_len - 9] = 'G';
    size[size_len - 8] = '\0';
    size_left_gap = size_header_len - (size_len - 8);
  }

  printw_str(win, size, x + size_left_gap, y);
}

static void draw_mod_time(WINDOW *win, PanelEntry *entry, int x, int y) {
  char *mod_time = entry->mod_time;
  int date_header_len = cols[MOD_TIME].width;
  int date_len = strlen(mod_time);

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
    memcpy(mod_time, &mod_time[offset], sizeof(char) * new_date_len);
    mod_time[new_date_len] = '\0';
  }

  printw_str(win, mod_time, x, y);
}

void panel_enter_dir(Panel *panel) {
  panel_load_sorted_dir(panel);
  panel->selected_item = 0;
  panel_draw_dir(panel);
  panel_toggle_highlight(panel, true);

  refresh_win(panel->win);
}

void static enter_by_vim(char *path) {
  def_prog_mode();
  endwin();

  char cmd[2048];
  snprintf(cmd, sizeof(cmd), "vim %s", path);
  int result = system(cmd);

  reset_prog_mode();
  return;
}

void panel_toggle_highlight(Panel *panel, bool highlight) {
  int selected_item = panel->selected_item;
  FileInfo *file_info = &panel->items[selected_item];

  PanelEntry panel_entry = {.name = file_info->name,
                            .file_type = file_info->file_type,
                            .is_selected = highlight};
  draw_name(panel->win, &panel_entry, 1, get_y(selected_item, PAGE_SIZE));

  refresh_win(panel->win);
}

void panel_move_selection(Panel *panel, int position) {
  int page = (panel->selected_item / PAGE_SIZE) + 1;
  if (panel == NULL || position > page * PAGE_SIZE - 1 ||
      position < PAGE_SIZE * (page - 1)) {
    return;
  }

  panel_toggle_highlight(panel, false);
  panel->selected_item = position;
  panel_toggle_highlight(panel, true);

  refresh_win(panel->win);
}

void panel_switch(Panel *old_panel, Panel *new_panel) {
  panel_toggle_highlight(old_panel, false);
  panel_toggle_highlight(new_panel, true);
  refresh_win(old_panel->win);
  refresh_win(new_panel->win);
}

int panel_get_index_dir_by_name(Panel *panel, char *dir_name) {
  for (int i = 0; i < panel->count; i++) {
    FileInfo *file_info = &panel->items[i];
    if (strcmp_(file_info->name, dir_name) == 0) {
      return i;
    }
  }

  return 0;
}

void panel_erase_dir_area(Panel *panel) {
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

void panel_draw_headers_names(Panel *panel) {
  WINDOW *win = panel->win;
  for (int i = 0; i < cols_size; i++) {
    ColumnDef *col = &cols[i];
    if (col->width > 0) {
      printw_str(win, col->header, get_header_x(col, cols), 1);
    }
  }
}

void panel_draw_headers_hborders(Panel *panel) {
  WINDOW *win = panel->win;
  int x = 1;
  int length = 0;

  for (int i = 0; i < cols_size; i++) {
    ColumnDef *col = &cols[i];

    if (col->width > 0) {
      int vborder_x = get_vborder_x(col, cols);
      printw_hline(win, x, 2, vborder_x);
      length += vborder_x;
    }
  }
}

void panel_draw_headers_vborders(Panel *panel) {
  WINDOW *win = panel->win;

  for (int i = 0; i < cols_size; i++) {
    ColumnDef *col = &cols[i];

    if (col->width != 0) {
      printw_vline(win, get_vborder_x(col, cols), 1, Y_BOTTOM_OFFSET);
    }
  }
}

void panel_draw_columns(Panel *panel) {
  panel_draw_headers_names(panel);
  panel_draw_headers_hborders(panel);
  panel_draw_headers_vborders(panel);

  refresh_win(panel->win);
}

void panel_draw_dir(Panel *panel) {

  WINDOW *win = panel->win;
  FileInfo *items = panel->items;

  int item_y = get_y(panel->selected_item, PAGE_SIZE);
  int start = panel->selected_item - (item_y - Y_TOP_OFFSET);
  int end = panel->selected_item + (Y_BOTTOM_OFFSET - item_y);

  panel_erase_dir_area(panel);

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

  for (int i = start; i <= end && i < panel->count; i++) {
    FileInfo *item = items + i;

    char *name = item->name;
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

    PanelEntry panel_entry = {.name = name,
                              .size = sizebuf,
                              .mod_time = datebuf,
                              .file_type = item->file_type};

    for (int i = 0; i < cols_size; i++) {
      ColumnDef *col = &cols[i];
      if (col->width > 0) {
        col->column_render(win, &panel_entry, get_col_draw_x(col, cols), y);
      }
    }
    y++;
  }

  refresh_win(win);
}

void panel_draw(Panel *panel) {
  panel_draw_columns(panel);
  panel_draw_dir(panel);
  refresh_win(panel->win);
}

void panel_reload(Panel *panel, bool highlight) {
  panel_load_sorted_dir(panel);
  panel_draw(panel);
  panel_toggle_highlight(panel, highlight);
  box(panel->win, 0, 0);
  refresh_win(panel->win);
}

void panel_refresh(Panel *panel) {
  panel_draw(panel);
  panel_toggle_highlight(panel, true);
  box(panel->win, 0, 0);
  refresh_win(panel->win);
}

void panel_exit_file(Panel *panel) {

  char entry_name[128];
  get_last_segment(panel->path, entry_name, sizeof(entry_name));

  if (strcmp(panel->path, entry_name) == 0) {
    panel_move_selection(panel, 0);
    refresh_win(panel->win);
    return;
  }

  substract_path_segment(panel->path, panel->path, 512);
  history_add(panel->history, panel->path);

  panel_load_sorted_dir(panel);
  panel->selected_item = 0;
  panel_draw(panel);

  panel->selected_item = panel_get_index_dir_by_name(panel, entry_name);
  panel_toggle_highlight(panel, true);

  refresh_win(panel->win);
}

void panel_enter_file(Panel *panel) {
  FileInfo *fileInfo = &panel->items[panel->selected_item];
  char *name = fileInfo->name;
  int type = fileInfo->file_type;

  if (strcmp(name, "..") == 0) {
    panel_exit_file(panel);
    return;
  }

  switch (type) {
  case FILE_TYPE_DIRECTORY:
    append_path_segment(panel->path, name, panel->path, 512);
    history_add(panel->history, panel->path);
    panel_enter_dir(panel);
    break;
  case FILE_TYPE_LNK_TO_DIR:
    append_path_segment(panel->path, name, panel->path, 512);
    char real_path[PATH_MAX];
    char *ptr = realpath(panel->path, real_path);
    if (ptr == NULL) {
      return;
    }
    strcpy(panel->path, real_path);
    panel_enter_dir(panel);
    history_add(panel->history, panel->path);
    break;
  case FILE_TYPE_EXECUTABLE_BINARY:
  case FILE_TYPE_EXECUTABLE_SCRIPT:
  case FILE_TYPE_BINARY_DATA:
    char path[PATH_MAX];
    append_path_segment(panel->path, name, path, PATH_MAX);
    enter_by_vim(path);
    break;
  }
}

void panel_load_sorted_dir(Panel *panel) {
  int elements = load_dir(panel->path, panel->items, 512);
  panel->count = elements;
  sort_dir(panel->items, panel->count);
}

void panel_scale_interface() {
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
