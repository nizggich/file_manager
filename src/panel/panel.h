#include "../fs/fs.h"
#include "../history/history.h"
#include "../popup/popup.h"
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
  History *history;
} Panel;

void panel_move_selection(Panel *panel, int position);
void panel_switch(Panel *old_panel, Panel *new_panel);
void panel_toggle_highlight(Panel *panel, bool highlight);

void panel_erase_dir_area(Panel *panel);
void panel_enter_file(Panel *panel);
void panel_enter_dir(Panel *panel);
void panel_exit_file(Panel *panel);
void panel_load_sorted_dir(Panel *panel);
int panel_get_index_dir_by_name(Panel *panel, char *dir_name);

void panel_draw(Panel *panel);
void panel_reload(Panel *panel, bool highlight);
void panel_refresh(Panel *panel);
void panel_draw_columns(Panel *panel);
void panel_draw_headers_names(Panel *panel);
void panel_draw_headers_hborders(Panel *panel);
void panel_draw_headers_vborders(Panel *panel);
void panel_draw_dir(Panel *panel);
void panel_scale_interface(void);
