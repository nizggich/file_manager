#include "commander.h"

static bool is_text_file(const char *path) {
	FILE *f = fopen(path, "rb");
	if (!f) return false;

	unsigned char buffer[512];
	size_t n = fread(buffer, 1, sizeof(buffer), f);
	fclose(f);

	for (size_t i = 0; i < n; i++) {
		if(buffer[i] == '\0') {
			return false;
		}
	}
	
	return true;
}
static int get_color_pair(FileType file_type) {

	switch(file_type) {
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


static FileType classify_file(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) {
	       return FILE_TYPE_BINARY_DATA;
	}	       	

	if (!S_ISREG(st.st_mode)) {
		return FILE_TYPE_DIRECTORY;
	}

	bool executable = (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0;
	bool is_text = is_text_file(path); 

	if (executable) {
		return is_text ? FILE_TYPE_EXECUTABLE_SCRIPT : FILE_TYPE_EXECUTABLE_BINARY;
	} else {
		return is_text ? FILE_TYPE_TEXT_PLAIN : FILE_TYPE_BINARY_DATA;
	}
}

static void load_dir(Panel *panel) {

	DIR *root = opendir(panel->path);
	struct dirent *fs_ent = NULL;
	struct stat sb;
	panel->count = 0;
	char dir_element_path[1024];

	while ((fs_ent = readdir(root)) != NULL && panel->count < MAX_FILES) 
	{
		char *name = fs_ent->d_name;
		int type = fs_ent->d_type;
		
		if (strcmp(name, ".") == 0 || (strcmp(panel->path, "/") == 0 && strcmp(name, "..") == 0))
	       	{
			continue;
		}	
		
		FileInfo *file_info = &panel->items[panel->count];
			
		snprintf(file_info->name, sizeof(file_info->name), "%s", name);
		append_path_segment(panel->path, name, file_info->path, 1024);
		
		file_info->mode = sb.st_mode;
		file_info->type = type;	

		if (stat(file_info->path, &sb) == 0) {
			file_info->mode = sb.st_mode;
			file_info->size = sb.st_size;
			file_info->mod_time = sb.st_mtim.tv_sec;

		}

		file_info->file_type = classify_file(file_info->path);

		panel->count++;
	}		

	closedir(root);
}

static int get_y(int selected_item) {
	int page = selected_item / PAGE_SIZE; 
	return selected_item - page * PAGE_SIZE + 3; 	
}

static void display_dir_item_iternal(WINDOW *win, char *name, FileType file_type, int y, bool highlight) {

	int color_pair = COLOR_PAIR(CP_SELECTED_ITEM);

	if (!highlight) {
		color_pair = get_color_pair(file_type); 
	} 	

	wattron(win, color_pair); 

	switch(file_type) {
		case FILE_TYPE_DIRECTORY: 
			mvwprintw(win, y, 1, "/%s", name);
			break;
		case FILE_TYPE_EXECUTABLE_BINARY:
			mvwprintw(win, y, 2, "@%s", name);
 			break;
		case FILE_TYPE_EXECUTABLE_SCRIPT:
			mvwprintw(win, y, 2, "*%s", name);
 			break;
		case FILE_TYPE_TEXT_PLAIN:
		case FILE_TYPE_BINARY_DATA:
			mvwprintw(win, y, 2, "%s", name);
 			break;
		}

	wattroff(win, color_pair);
}

static void display_ui(Panel *panel) {
	WINDOW *win = panel->win;
	wclear(win);
	box(win, 0, 0);

	int max_x = getmaxx(win);
	int max_y = getmaxy(win);

	mvwprintw(win, 1, NAME_HBORDER(max_x) / 2 - 1, "%s", "Name");
	mvwprintw(win, 1, SIZE_HBORDER(max_x) - SIZE_COL_WIDTH / 2 - 2, "%s", "Size"); 
	mvwprintw(win, 1, DATE_HBORDER(max_x) - DATE_COL_WIDTH / 2 - 2 , "%s", "Date");
	
	for (int i = 1; i < max_x; i++) {			
		if (i != max_x - DATE_COL_WIDTH - SIZE_COL_WIDTH && i != max_x - DATE_COL_WIDTH && i != max_x - 1) {
			mvwaddch(win, 2, i, ACS_HLINE);
		}		
	}

	for (int i = 0; i < LINES - 2; i++) {	
		mvwaddch(win, i + 1, NAME_HBORDER(max_x), ACS_VLINE);
		mvwaddch(win, i + 1, SIZE_HBORDER(max_x), ACS_VLINE);	
		mvwaddch(win, i + 1, DATE_HBORDER(max_x), ACS_VLINE);
	
	}

	wrefresh(win);
}

static void clear_dir(WINDOW *win) {	
	int max_x = getmaxx(win);

	for (int i = Y_OFFSET; i < LINES - 1; i++) {
		mvwhline(win, i, 1, ' ', NAME_HBORDER(max_x) - 1);
		mvwhline(win, i, NAME_HBORDER(max_x) + 1, ' ', SIZE_HBORDER(max_x) - NAME_HBORDER(max_x) - 1); 
		mvwhline(win, i, SIZE_HBORDER(max_x) + 1, ' ', DATE_HBORDER(max_x) - SIZE_HBORDER(max_x) - 1);
	}
}

static void display_dir(Panel *panel) {	
	WINDOW *win = panel->win;

	clear_dir(win);

	struct tm tm;
	char timebuf[64];
	char datebuf[12];
	
	int max_x = getmaxx(win);
	int max_y = getmaxy(win);

	int y = Y_OFFSET; 	

	for (int i = panel->selected_item; i < panel->count; i++) {

		if (y >= LINES - 1) {
			break;
		}

		int type = panel->items[i].type;		
		mode_t mode = panel->items[i].mode;

		display_dir_item_iternal(panel->win, panel->items[i].name, panel->items[i].file_type, y, false);
				
		int size = panel->items[i].size;
		time_t time = panel->items[i].mod_time;

		localtime_r(&time, &tm);
		strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);

		snprintf(datebuf, sizeof(datebuf), "%d", size);	
		
		wattron(win, COLOR_WHITE | A_BOLD);
		mvwprintw(win, y, SIZE_HBORDER(max_x) - SIZE_COL_WIDTH / 2 - strlen(datebuf) / 2, "%d", size);
      		mvwprintw(win, y, DATE_HBORDER(max_x) - DATE_COL_WIDTH / 2 - strlen(timebuf) / 2, "%s", timebuf);	
		wattroff(win, COLOR_WHITE | A_BOLD);

		y++;
	}

	wnoutrefresh(win);
}

static void display_dir_item(Panel *panel, int selected_item, bool highlight) {
	int y = get_y(selected_item);

	FileInfo *file_info = &panel->items[selected_item];

	display_dir_item_iternal(panel->win, file_info->name, file_info->file_type, y, highlight);	

	wnoutrefresh(panel->win);

}

static void move_selection(Panel *panel, int position) {	
	int page = (panel->selected_item / PAGE_SIZE) + 1;
	if (panel == NULL || position > page * PAGE_SIZE - 1 || position <  PAGE_SIZE * (page - 1)) {
		return;
	}

	int old_selection = panel->selected_item;
	int new_selection = position;
			
	display_dir_item(panel, old_selection, false);	
	display_dir_item(panel, new_selection, true);	
	
	wnoutrefresh(panel->win);
}

static void switch_panel(Panel *old_panel, Panel *new_panel) {
	display_dir_item(old_panel, old_panel->selected_item, false);
	display_dir_item(new_panel, new_panel->selected_item, true);	

	wnoutrefresh(old_panel->win);
	wnoutrefresh(new_panel->win);
}

static bool is_dir(const FileInfo *file_info) {
	if (file_info->mode != 0 && S_ISDIR(file_info->mode)) 
	{
		return true;
	}
	return false;
}


static int cmp_dir(const void *a, const void *b) {
	FileInfo *a_ent = (FileInfo*) a;
	FileInfo *b_ent = (FileInfo*) b;
	
	if (is_dir(a_ent) && is_dir(b_ent)) {
		return strcmp_(a_ent->name, b_ent->name);
	}
	else if (!is_dir(a_ent) && is_dir(b_ent)) {
		return 1;
	}
	else if (is_dir(a_ent) && !is_dir(b_ent)) { 
		return -1;
	}
	else if (!is_dir(a_ent) && !is_dir(b_ent)) {
		return strcmp_(a_ent->name, b_ent->name);
	}	
}


static void sort_dir(Panel *panel) {
	qsort_(panel->items, panel->count, sizeof(FileInfo), cmp_dir);
}

static int get_index_dir_by_name(Panel *panel, char *dir_name) {
	for (int i = 0; i < panel->count; i++) {
		FileInfo *file_info = &panel->items[i];
		if (strcmp_(file_info->name, dir_name) == 0) {
				return i;
		}	
	}

	return 0;
}

static void exit_dir(Panel *panel) {

	char entry_name[128];
	get_last_segment(panel->path, entry_name, sizeof(entry_name));

	substract_path_segment(panel->path, panel->path, MAX_PATH);
	panel->selected_item = 0;
	panel->count = 0;
	load_dir(panel);
	sort_dir(panel);		
	display_dir(panel);
	panel->selected_item = get_index_dir_by_name(panel, entry_name);
	move_selection(panel, panel->selected_item);
}

static void enter_dir(Panel *panel) {
	FileInfo *fileInfo = &panel->items[panel->selected_item];	
	char *name = fileInfo->name;
	int type = fileInfo->file_type;

	if (type == FILE_TYPE_DIRECTORY && panel->count > 0) { 
			
		if (strcmp_(name, "..") == 0) {
			exit_dir(panel);		
			return;
		}	

		char result[2048];//TO DO: check append_path_segment
		strcpy(result, panel->path);
		append_path_segment(panel->path, fileInfo->name, result, MAX_PATH);
		strcpy(panel->path, result);
		
		panel->selected_item = 0;	
		panel->count = 0;
		load_dir(panel);	
		sort_dir(panel);
		display_dir(panel);
		move_selection(panel, panel->selected_item);

	} else {
		def_prog_mode();
		endwin();

		char cmd[2048];
		snprintf(cmd, sizeof(cmd), "vim %s/%s", panel->path, fileInfo->name);
		int result = system(cmd);

		reset_prog_mode();
		refresh();	
	}

}

void commander_run() {
	initscr();
	
	if (has_colors() == false) {
		endwin();
		printw("Your terminal does not support colors");
		exit(1);
	}

	cbreak();
	noecho();
	keypad(stdscr, TRUE);	
	start_color();
	use_default_colors();
	curs_set(0);

	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_WHITE);

	
	clear();
	refresh();

	int height, width;
	getmaxyx(stdscr, height, width);	
	WINDOW *left_win = newwin(LINES, COLS / 2, 0, 0);
	WINDOW *right_win = newwin(LINES, COLS / 2, 0, COLS / 2); 	
	
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

	load_dir(&left_panel);
	load_dir(&right_panel);
	sort_dir(&left_panel);
	sort_dir(&right_panel);
        
	box(left_win, 0, 0);
	box(right_win, 0, 0);
	display_ui(&left_panel);
	display_ui(&right_panel);
	display_dir(&left_panel);
	move_selection(&left_panel, left_panel.selected_item);
	display_dir(&right_panel);
	doupdate();

        char ch;
	int activePanel = 0;

	while((ch = getch()) != 'q')	
	{
		Panel *panel = panels[activePanel];

		if (ch == 119 && panel->selected_item > 0) {//w
			if (get_y(panel->selected_item) == Y_OFFSET) {
			     panel->selected_item = panel->selected_item - PAGE_SIZE;
		     	     display_dir(panel);
			     panel->selected_item = panel->selected_item + (PAGE_SIZE - 1) / 2;
			     move_selection(panel, panel->selected_item);
			} else {	     
				move_selection(panel, panel->selected_item - 1);
				panel->selected_item--;
			}
		}
		else if (ch == 115 && panel->selected_item < panel->count - 1) {//s
			if (get_y(panel->selected_item) == LINES - 2) {
				panel->selected_item++;
				display_dir(panel);
				panel->selected_item = panel->selected_item + (PAGE_SIZE - 1) / 2;
				move_selection(panel, panel->selected_item);
			} else {
				move_selection(panel, panel->selected_item + 1);	
				panel->selected_item++;
			}
		}
		else if (ch == 9)  { //Tab
			if (activePanel == PANEL_COUNT - 1)
				activePanel = 0;
			else 
				activePanel++;

			panel->active = false;
			panels[activePanel]->active = true;

			switch_panel(panel, panels[activePanel]);
		}
		else if (ch == 10) {//Enter 
		
			enter_dir(panel);
		}
		else if (ch == 7) {//Backspace
				   
			exit_dir(panel);
		}	

		doupdate();
	}
	
	endwin();
}
