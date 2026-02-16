#include "commander.h"

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
		
		if (strcmp(name, ".") == 0)
	       	{
			continue;
		}	
		
		snprintf(panel->items[panel->count].name, sizeof(panel->items[panel->count].name), "%s", name);
		panel->items[panel->count].mode = sb.st_mode;
		panel->items[panel->count].size = 0;
		panel->items[panel->count].mod_time = 0;
		panel->items[panel->count].type = type;
		append_path_segment(panel->path, name, dir_element_path, 1024);

		if (stat(dir_element_path, &sb) == 0) {
			panel->items[panel->count].mode = sb.st_mode;
			panel->items[panel->count].size = sb.st_size;
			panel->items[panel->count].mod_time = sb.st_mtim.tv_sec;


		}

		panel->count++;
	}		

	closedir(root);
}

static int get_y(int selected_item) {
	int page = selected_item / PAGE_SIZE; 
	return selected_item - page * PAGE_SIZE + 3; 	
}

static void display_dir_item_iternal(WINDOW *win, char *name, int type, int y, bool highlight) {

	if (highlight) {
		wattron(win, COLOR_PAIR(1));
	}
	
	if (type == DT_DIR || type == DT_LNK) {		
		mvwprintw(win, y, 1, "/%s", name);	
	}
	else {	
		mvwprintw(win, y, 2, "%s", name);				
	}
	
	wattroff(win, COLOR_PAIR(1));
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

		display_dir_item_iternal(panel->win, panel->items[i].name, type, y, false);
				
		int size = panel->items[i].size;
		time_t time = panel->items[i].mod_time;

		localtime_r(&time, &tm);
		strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);

		snprintf(datebuf, sizeof(datebuf), "%d", size);	

		mvwprintw(win, y, SIZE_HBORDER(max_x) - SIZE_COL_WIDTH / 2 - strlen(datebuf) / 2, "%d", size);
      		mvwprintw(win, y, DATE_HBORDER(max_x) - DATE_COL_WIDTH / 2 - strlen(timebuf) / 2, "%s", timebuf);	

		y++;
	}

	wnoutrefresh(win);
}

static void display_dir_item(Panel *panel, int selected_item, bool highlight) {
	int y = get_y(selected_item);
	FileInfo *file_info = &panel->items[selected_item];
	display_dir_item_iternal(panel->win, file_info->name, file_info->type, y, highlight);	
}

static void move_selection(Panel *panel, Direction direction) {	
	int old_selection = panel->selected_item;
	int new_selection = 0;

	switch(direction) {
		case UP:
			new_selection = old_selection - 1;
			break;
		case DOWN:
			new_selection = old_selection + 1;
			break;
		default:
			new_selection = old_selection;
			break;
	}
			
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
	curs_set(0);

	init_pair(1, COLOR_BLUE, COLOR_WHITE);
	init_pair(2, COLOR_WHITE, COLOR_WHITE);

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
        
       	refresh();	
	box(left_win, 0, 0);
	box(right_win, 0, 0);
	display_ui(&left_panel);
	display_ui(&right_panel);
	display_dir(&left_panel);
	move_selection(&left_panel, IN_PLACE);
	display_dir(&right_panel);
	wrefresh(left_win);
	wrefresh(right_win);

        char ch;
	int activePanel = 0;
	//wclear(left_win);
//	wclear(right_win);  //TAB = 9
//	int y = get_y(47);
//	  while ((ch = getch()) != 'q') {
//		attron(COLOR_PAIR(1));
//		mvwprintw(stdscr, 49, 0, "%s\n", "!");
//		attroff(COLOR_PAIR(1));
//		//mvwprintw(right_win, 15, 20, "%d\n", ch);
//		wrefresh(left_win);
//		wrefresh(right_win);
//}

	while((ch = getch()) != 'q')	
	{
		Panel *panel = panels[activePanel];

		if (ch == 119 && panel->selected_item > 0) {//w
			if (get_y(panel->selected_item) == Y_OFFSET) {
			     panel->selected_item = panel->selected_item - PAGE_SIZE;
		     	     display_dir(panel);
			     panel->selected_item = panel->selected_item + PAGE_SIZE - 1;
			     move_selection(panel, IN_PLACE);
			} else {	     
				move_selection(panel, UP);
				panel->selected_item--;
			}
		}
		else if (ch == 115 && panel->selected_item < panel->count - 1) {//s
			if (get_y(panel->selected_item) == LINES - 2) {
				panel->selected_item++;
				display_dir(panel);
				move_selection(panel, IN_PLACE);
			} else {
				move_selection(panel, DOWN);	
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
		
			FileInfo fileInfo = panel->items[panel->selected_item];	
			int type = fileInfo.type;

			if ((type == DT_DIR || type == DT_LNK) && panel->count > 0) {
				
				char result[2048];//what wrong with append_path_segment	
				strcpy(result, panel->path);
				append_path_segment(panel->path, fileInfo.name, result, MAX_PATH);
				strcpy(panel->path, result);

			  	panel->selected_item = 0;	
				load_dir(panel);	
				sort_dir(panel);
				display_dir(panel);
				move_selection(panel, IN_PLACE);
			} 
			else if (type == DT_REG) {
				def_prog_mode();
				endwin();

				char cmd[2048];
				snprintf(cmd, sizeof(cmd), "vim %s/%s", panel->path, fileInfo.name);
				int result = system(cmd);

				reset_prog_mode();
				refresh();	
				}
		}
		else if (ch == 7) {//Backspace
			substract_path_segment(panel->path, panel->path, MAX_PATH);
			panel->selected_item = 0;//make return to same index	
			load_dir(panel);
			sort_dir(panel);		
			display_dir(panel);
			move_selection(panel, IN_PLACE);
		}	
		doupdate();
	}
	
	endwin();
}
