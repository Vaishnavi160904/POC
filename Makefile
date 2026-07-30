# Makefile for HR Resume Screening System
# Works with MinGW-w64 (gcc/mingw32-make) on Windows, and gcc/make on Linux/Mac.

CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC_DIR = src
OBJ_DIR = obj
BIN = hr_resume_screener.exe

SOURCES = $(wildcard $(SRC_DIR)/*.c) main.c
OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SOURCES)))

VPATH = $(SRC_DIR)

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

run: all
	./$(BIN)

clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all run clean
