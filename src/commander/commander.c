#include "commander.h"

static fs_ent_info fs_ent_infs[2048];
static int fs_ent_index = 0;
static char current_path[PATH_MAX];
static int selected = 0;

static void load_dir(const char *path) {

	DIR *root = opendir(path);
	struct dirent *fs_ent = NULL;
	struct stat sb;

	while ((fs_ent = readdir(root)) != NULL) 
	{
		char *name = fs_ent->d_name;
		int type = fs_ent->d_type;
		
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
	       	{
			continue;
		}

		fs_ent_info fs_ent = {0};
		snprintf(fs_ent.name, sizeof(fs_ent.name), "%s", name);
		fs_ent.type = type; 	
		append_path_segment(path, fs_ent.name, fs_ent.path, sizeof(fs_ent.path));
		
		if (stat(fs_ent.path, &sb) == 0) {
			fs_ent.mode = sb.st_mode;	
			fs_ent.size = sb.st_size;
			fs_ent.mod_time = sb.st_mtim.tv_sec;
		}

		fs_ent_infs[fs_ent_index++] = fs_ent;
	}		

	snprintf(current_path, sizeof(current_path),"%s", path); 		

	closedir(root);
}

static void display_dir(WINDOW *win) {	
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
	
	for (int i = 0; i < fs_ent_index; i++) {
		fs_ent_info *fs_ent = fs_ent_infs + i;	
		int type = fs_ent->type;

		if ( i == selected) {
			attron(COLOR_PAIR(1));
		}

		if (type == DT_DIR || type == DT_LNK) {		
			mvwprintw(win, i + 3, 1, "/%s", fs_ent->name);	
		}
		else {	
			mvwprintw(win, i + 3, 2, "%s", fs_ent->name);		
			
		}

		attroff(COLOR_PAIR(1));

		int size = fs_ent_infs[i].size;
		time_t time = fs_ent_infs[i].mod_time;

		localtime_r(&time, &tm);
		strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);

		snprintf(datebuf, sizeof(datebuf), "%d", size);	

		mvwprintw(win, i + 3, SIZE_HBORDER(x) - SIZE_COL_WIDTH / 2 - strlen(datebuf) / 2, "%d", size);
      		mvwprintw(win, i + 3, DATE_HBORDER(x) - DATE_COL_WIDTH / 2 - strlen(timebuf) / 2, "%s", timebuf);	
	}

	wrefresh(win);
}

static bool is_dir(const fs_ent_info *fs_ent) {

	if (fs_ent->mode != 0 && S_ISDIR(fs_ent->mode)) 
	{
		return true;
	}
	return false;
}

static int cmp_dir(const void *a, const void *b) {
	fs_ent_info *a_ent = (fs_ent_info*) a;
	fs_ent_info *b_ent = (fs_ent_info*) b;
	
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

static void sort_dir() {
	qsort_(fs_ent_infs, fs_ent_index, sizeof(fs_ent_info), cmp_dir);
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

	char cwd[PATH_MAX];
	
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
	       cwd[0] = '/';
	       cwd[1] = '\0';
	}	      

	load_dir(cwd);
	sort_dir();
        
       	refresh();	
	box(left_win, 0, 0);
	box(right_win, 0, 0);
	display_dir(left_win);
	display_dir(right_win);
	wrefresh(left_win);
	wrefresh(right_win);

        char ch;

	while((ch = getch()) != 'q')	
	{
		if (ch == 119 && selected > 0) {//s
			selected--;
		}
		else if (ch == 115 && selected < fs_ent_index - 1) {//w
			selected++;
		}
		else if (ch == 10) {//Enter 
				    
			fs_ent_info *fs_ent = fs_ent_infs + selected;
			int type = fs_ent->type;

			if ((type == DT_DIR || type == DT_LNK) && fs_ent_index > 0) {
			   	
				char result_path[256];
			       	append_path_segment(current_path, fs_ent_infs[selected].name, result_path, 256);
				
				fs_ent_index = 0;
				load_dir(result_path);	
				sort_dir();
				selected = 0;	
			} 
			else if (type == DT_REG) {
				def_prog_mode();
				endwin();

				char cmd[2048];
			   	snprintf(cmd, sizeof(cmd), "vim %s%s", current_path, fs_ent->name);
				int result = system(cmd);

				reset_prog_mode();
				refresh();	
			}
		}
		else if (ch == 7) {//Backspace
			char result_path[256];
			substract_path_segment(current_path, result_path, 256);
		
			fs_ent_index = 0;	
			load_dir(result_path);
			sort_dir();		
			selected = 0;	
		}
	
		display_dir(left_win);
		display_dir(right_win);
	}
	
	endwin();
}
