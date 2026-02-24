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

typedef enum 
{
	CP_DIR = 1,
	CP_BIN_DATA = 1,
	CP_EXE_SCR = 2,
	CP_EXE_BIN = 3,
	CP_SELECTED_ITEM = 4
} ColorPair;

typedef enum {
	FILE_TYPE_DIRECTORY,
	FILE_TYPE_EXECUTABLE_SCRIPT,
	FILE_TYPE_EXECUTABLE_BINARY,
	FILE_TYPE_BINARY_DATA,
	FILE_TYPE_TEXT_PLAIN
} FileType;


typedef struct {
	char name[128];	
	char path[512];
	unsigned int type;
	mode_t mode;
	off_t size;
	time_t mod_time;	
	FileType file_type;
} FileInfo;

typedef struct {
	char path[512];//mb pointer a ne sam massiv
	FileInfo items[2048];	
	bool active;
	int count;
	int selected_item;
	char last_visited_dir[128];
	WINDOW *win;
} Panel;

void commander_run();




