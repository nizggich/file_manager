#include "../fs/fs.h"
#include "../nav_history/NavHistory.h"
#include "../ui/ui.h"
#include "../utils/utils.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

extern int term_width, term_height;

#define PAGE_SIZE (term_height - 4)
#define Y_TOP_OFFSET 3
#define Y_BOTTOM_OFFSET (term_height - 2)
#define HISTORY_SIZE 50

typedef enum { NAME, SIZE, MOD_TIME } ColumnType;

typedef struct {
  char *name;
  char *size;
  char *mod_time;
  FileType file_type;
  bool is_selected;
} PanelEntry;

typedef void (*ColumnRender)(WINDOW *, PanelEntry *, int, int);

typedef struct {
  char *header;
  int desired_width;
  int min_width;
  int width;
  int max_width;
  int weight;
  ColumnRender column_render;
} ColumnDef;

typedef struct {
  char path[512]; // mb pointer a ne sam massiv
  FileInfo items[2048];
  bool active;
  int count;
  int selected_item;
  WINDOW *win;
  NavHistory *history;
} Panel;

void move_selection(Panel *panel, int position);
void switch_panel(Panel *old_panel, Panel *new_panel);
void toggle_highlight(Panel *panel, bool highlight);

void erase_dir_area(Panel *panel);
void enter_file(Panel *panel);
void enter_dir(Panel *panel);
void exit_file(Panel *panel);
void load_sorted_dir(Panel *panel);
WINDOW *create_popup_win(char *title, int height, int width, int x, int y);
WINDOW *create_alert_dialog(char *title, int height, int width, int x, int y);
void handle_win_input(WINDOW *win, char *input_buf, int size, bool alert_dialog,
                      bool display);
void wait_input(WINDOW *win);
int get_index_dir_by_name(Panel *panel, char *dir_name);

void draw_panel(Panel *panel);
void reload_panel(Panel *panel, bool highlight);
void refresh_panel(Panel *panel);
void draw_columns(Panel *panel);
void draw_headers_names(Panel *panel);
void draw_headers_hborders(Panel *panel);
void draw_headers_vborders(Panel *panel);
void draw_dir(Panel *panel);
void scale_interface(void);
