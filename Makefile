#Makefile

CC = gcc
LIBS = ncurses

SRCS = src/main.c src/commander/commander.c src/qsort/qsort.c src/utils/utils.c
TARGET = commander_

.PHONY: build run clean

build:
	$(CC) $(SRCS) -l$(LIBS) -g -O0 -o $(TARGET)
 
debug:build
	gdb ./$(TARGET)
	
run: build	
	./$(TARGET)

clean:
	rm -f $(TARGET)

