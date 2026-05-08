#include "../fs/fs.h"
#include "../ui/ui.h"
#include "../utils/utils.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

extern int term_width, term_height;

#define PAGE_SIZE (term_height - 4)

#define Y_TOP_OFFSET 3
#define Y_BOTTOM_OFFSET (term_height - 2)

typedef enum { NAME, SIZE, MOD_TIME } ColumnType;

typedef struct {
  char *header;
  int desired_width;
  int min_width;
  int width;
  int max_width;
  int weight;
} ColumnDef;

typedef struct {
  char path[512]; // mb pointer a ne sam massiv
  FileInfo items[2048];
  bool active;
  int count;
  int selected_item;
  WINDOW *win;
} Panel;

void move_selection(Panel *panel, int position);
void switch_panel(Panel *old_panel, Panel *new_panel);

void erase_dir_area(Panel *panel);
void exit_dir(Panel *panel);
void enter_dir(Panel *panel);
int get_index_dir_by_name(Panel *panel, char *dir_name);

void draw_ui(Panel *panel);
void draw_headers_names(Panel *panel);
void draw_headers_hborders(Panel *panel);
void draw_headers_vborders(Panel *panel);
void draw_dir(Panel *panel);
void scale_interface(void);
