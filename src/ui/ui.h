#include <ncurses.h> 

typedef enum 
{
	CP_DIR = 1,
	CP_BIN_DATA = 1,
	CP_EXE_SCR = 2,
	CP_EXE_BIN = 3,
	CP_SELECTED_ITEM = 4
} ColorPair;

void erase_area(WINDOW *win, int x1, int x2, int y1, int y2); 

void printw_str(WINDOW *win, char *val, int x, int y, char *format);
void printw_int(WINDOW *win, int val, int x, int y);
//void printwf(WINDOW *win, char *val, int x, int y); 
void printw_vline(WINDOW *win, int x, int y, int lines);
void printw_hline(WINDOW *win, int x, int y, int lines);

void color_on(WINDOW *win, int attr);
void color_off(WINDOW *win, int attr);

int get_max_x(WINDOW *win);
int get_max_y(WINDOW *win);

void refresh_win(WINDOW *win);

