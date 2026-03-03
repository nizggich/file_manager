#include "../fs/fs.h"
#include "../ui/ui.h"
#include "../utils/utils.h"
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define PAGE_SIZE (LINES - 4)

#define Y_TOP_OFFSET 3
#define Y_BOTTOM_OFFSET (LINES - 2)

#define NAME_COL_WIDTH 30
#define SIZE_COL_WIDTH 15 
#define DATE_COL_WIDTH 26 

#define NAME_HBORDER(columns) (columns - DATE_COL_WIDTH - SIZE_COL_WIDTH)
#define SIZE_HBORDER(columns) (columns - DATE_COL_WIDTH)
#define DATE_HBORDER(columns) (columns - 1)



typedef struct {
	char path[512];//mb pointer a ne sam massiv
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

void display_ui(Panel *panel); 
void display_headers_hborders(Panel *panel); 
void display_headers_vborders(Panel *panel); 
void display_dir(Panel *panel);
void display_dir_item(Panel *panel, int selected_item, bool highlight); 


