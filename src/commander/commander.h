#include "../qsort/qsort.h"
#include <time.h>
#include <dirent.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define NAME_COL_WIDTH 30
#define SIZE_COL_WIDTH 15 
#define DATE_COL_WIDTH 26 

#define NAME_HBORDER(columns) (columns - DATE_COL_WIDTH - SIZE_COL_WIDTH)
#define SIZE_HBORDER(columns) (columns - DATE_COL_WIDTH)
#define DATE_HBORDER(columns) (columns - 1)

typedef struct {
	char name[128];
	char path[1024];
	unsigned int type;
	mode_t mode;
	off_t size;
	time_t mod_time;	
} fs_ent_info;

void commander_run();




