#include "../qsort/qsort.h"
#include <time.h>
#include <dirent.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define PANEL_COUNT 2

#define PAGE_SIZE (LINES - 4)

#define Y_OFFSET 3

#define MAX_PATH 2048 
#define MAX_FILES 2048

#define NAME_COL_WIDTH 30
#define SIZE_COL_WIDTH 15 
#define DATE_COL_WIDTH 26 

#define NAME_HBORDER(columns) (columns - DATE_COL_WIDTH - SIZE_COL_WIDTH)
#define SIZE_HBORDER(columns) (columns - DATE_COL_WIDTH)
#define DATE_HBORDER(columns) (columns - 1)

typedef struct {
	char name[128];	
	unsigned int type;
	mode_t mode;
	off_t size;
	time_t mod_time;	
} FileInfo;

typedef struct {
	char path[MAX_PATH];//mb pointer a ne sam massiv
	FileInfo items[MAX_FILES];	
	bool active;
	int count;
	int selected_item;
	WINDOW *win;
} Panel;

typedef enum {
	UP,
	DOWN,
	IN_PLACE	
} Direction;

void commander_run();




