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

static void display_dir() {	
	mvprintw(0, NAME_HBORDER(COLS) / 2 - 2, "%s", "Name");
	mvprintw(0, SIZE_HBORDER(COLS) - ENT_SIZE_WIDTH / 2 - 2, "%s", "Size");
      	mvprintw(0, DATE_HBORDER(COLS) - DATE_WIDTH / 2 - 2, "%s", "Date");	

	for (int i = 0; i < LINES; i++) {
		mvaddch(i, NAME_HBORDER(COLS), ACS_VLINE);
		mvaddch(i, SIZE_HBORDER(COLS), ACS_VLINE);	
		mvaddch(i, DATE_HBORDER(COLS), ACS_VLINE);
	
	}
	
	for (int i = 0; i < fs_ent_index; i++) {
		fs_ent_info *fs_ent = fs_ent_infs + i;	
		int type = fs_ent->type;

		if ( i == selected) {
			attron(COLOR_PAIR(1));
		}

		if (type == DT_DIR || type == DT_LNK) {		
			mvprintw(i + 2, 0, "/%s", fs_ent->name);	
		}
		else {	
			mvprintw(i + 2, 1, "%s", fs_ent->name);		
			
		}

		attroff(COLOR_PAIR(1));

	}

	for (int i = 0; i < COLS / 2; i++) {	
		if (i != COLS / 2 - DATE_WIDTH - ENT_SIZE_WIDTH && i != COLS / 2 - DATE_WIDTH) {
			mvaddch(1, i, ACS_HLINE);
		}		
	}

	struct tm tm;
	char timebuf[64];
	char datebuf[12];
	for (int i = 0; i < fs_ent_index; i++) {	

		int size = fs_ent_infs[i].size;
		time_t time = fs_ent_infs[i].mod_time;

		localtime_r(&time, &tm);
		strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);

		snprintf(datebuf, sizeof(datebuf), "%d", size);	

		mvprintw(i + 2, SIZE_HBORDER(COLS) - ENT_SIZE_WIDTH / 2 - strlen(datebuf) / 2, "%d", size);
      		mvprintw(i + 2, DATE_HBORDER(COLS) - DATE_WIDTH / 2 - strlen(timebuf) / 2, "%s", timebuf);	
	}
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
	
	char cwd[PATH_MAX];
	
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
	       cwd[0] = '/';
	       cwd[1] = '\0';
	}	      

	load_dir(cwd);
	sort_dir();
	display_dir();

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

		clear();	
		display_dir();
		refresh();
	}
	
	endwin();
}
