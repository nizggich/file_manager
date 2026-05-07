#Makefile

CC = gcc
LIBS = ncurses

SRCS = src/main.c src/emperror/emperror.c src/fs/fs.c src/ui/ui.c src/panel/panel.c src/qsort/qsort.c src/utils/utils.c
TARGET = emperror 

.PHONY: build run clean

build:
	$(CC) $(SRCS) -l$(LIBS) -g -O0 -o $(TARGET)
 
debug:build
	gdb -q ./$(TARGET) 
	
run: build	
	./$(TARGET)

clean:
	rm -f $(TARGET)

