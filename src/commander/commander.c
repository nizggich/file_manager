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
		
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
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

static void display_dir(WINDOW *win, Panel *panel) {	
	wclear(win);
	box(win, 0, 0);

	int x = getmaxx(win);
	int y = getmaxy(win);

	mvwprintw(win, 1, NAME_HBORDER(x) / 2 - 1, "%s", "Name");
	mvwprintw(win, 1, SIZE_HBORDER(x) - SIZE_COL_WIDTH / 2 - 2, "%s", "Size"); 
	mvwprintw(win, 1, DATE_HBORDER(x) - DATE_COL_WIDTH / 2 - 2 , "%s", "Date");
	
	for (int i = 1; i < x; i++) {			
		if (i != x - DATE_COL_WIDTH - SIZE_COL_WIDTH && i != x - DATE_COL_WIDTH && i != x - 1) {
			mvwaddch(win, 2, i, ACS_HLINE);
		}		
	}

	for (int i = 0; i < LINES - 2; i++) {	
		mvwaddch(win, i + 1, NAME_HBORDER(x), ACS_VLINE);
		mvwaddch(win, i + 1, SIZE_HBORDER(x), ACS_VLINE);	
		mvwaddch(win, i + 1, DATE_HBORDER(x), ACS_VLINE);
	
	}

	struct tm tm;
	char timebuf[64];
	char datebuf[12];
	
	for (int i = 0; i < panel->count; i++) {

		int type = panel->items[i].type;

		if ( i == panel->selectedItem) {
			wattron(win, COLOR_PAIR(1));
		}

		if (type == DT_DIR || type == DT_LNK) {		
			mvwprintw(win, i + 3, 1, "/%s", panel->items[i].name);	
		}
		else {	
			mvwprintw(win, i + 3, 2, "%s", panel->items[i].name);		
			
		}

		wattroff(win, COLOR_PAIR(1));

		int size = panel->items[i].size;
		time_t time = panel->items[i].mod_time;

		localtime_r(&time, &tm);
		strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);

		snprintf(datebuf, sizeof(datebuf), "%d", size);	

		mvwprintw(win, i + 3, SIZE_HBORDER(x) - SIZE_COL_WIDTH / 2 - strlen(datebuf) / 2, "%d", size);
      		mvwprintw(win, i + 3, DATE_HBORDER(x) - DATE_COL_WIDTH / 2 - strlen(timebuf) / 2, "%s", timebuf);	
	}

	wnoutrefresh(win);
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
	left_panel.active = true;
	right_panel.selectedItem = -1;

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
	display_dir(left_win, &left_panel);
	display_dir(right_win, &right_panel);
	wrefresh(left_win);
	wrefresh(right_win);

        char ch;
	int activePanel = 0;
//     wclear(left_win);
//     wclear(right_win);  //TAB = 9
//     while ((ch = getch()) != 'q') {
//	attron(COLOR_PAIR(1));
//     	mvwprintw(stdscr, 15, 15, "%s\n", "HELLO");
//	attroff(COLOR_PAIR(1));
//     	mvwprintw(right_win, 15, 16, "%d\n", ch);
//     	wrefresh(left_win);
//     	wrefresh(right_win);
//     }
//
	while((ch = getch()) != 'q')	
	{
		Panel *panel = panels[activePanel];

		if (ch == 119 && panel->selectedItem > 0) {//w
			panel->selectedItem--;
		}
		else if (ch == 115 && panel->selectedItem < panel->count - 1) {//s
			panel->selectedItem++;
		}
		else if (ch == 9)  { //Tab
			if (activePanel == PANEL_COUNT - 1)
				activePanel = 0;
			else 
				activePanel++;

			panel->active = false;
			panel->selectedItem = -1;	

			panels[activePanel]->active = true;
			panels[activePanel]->selectedItem = 0;	
		}
		else if (ch == 10) {//Enter 
		
			FileInfo fileInfo = panel->items[panel->selectedItem];	
			int type = fileInfo.type;

			if ((type == DT_DIR || type == DT_LNK) && panel->count > 0) {
				
				char result[2048];//what wrong with append_path_segment	
				strcpy(result, panel->path);
				append_path_segment(panel->path, fileInfo.name, result, MAX_PATH);
				strcpy(panel->path, result);

			  	panel->selectedItem = 0;	
				load_dir(panel);	
				sort_dir(panel);
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
			panel->selectedItem = 0;//make return to same index	
			load_dir(panel);
			sort_dir(panel);		
		}	
		display_dir(left_win, &left_panel);
		display_dir(right_win, &right_panel);
		doupdate();
	}
	
	endwin();
}
