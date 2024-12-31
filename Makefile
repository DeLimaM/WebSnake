CC = gcc
CFLAGS = -Wall -Wextra -I./include
LDFLAGS = -lwebsockets -ljson-c -lpthread

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
BIN_DIR = /usr/bin/websnake

SOURCES = $(shell find $(SRC_DIR) -name '*.c')
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/websnake

all: build

build: create_dirs $(TARGET)

create_dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/game
	mkdir -p $(BUILD_DIR)/server

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build

install:
	@[ -f $(TARGET) ] || (echo "Please build the program first by running 'make build'" && exit 1)
	sudo cp $(TARGET) $(BIN_DIR)

uninstall:
	sudo rm -f $(BIN_DIR)

reinstall: uninstall rebuild install

help:
	@echo "Available targets:"
	@echo "  build     - Build the snake game"
	@echo "  clean     - Remove build files"
	@echo "  rebuild   - Clean and rebuild"
	@echo "  install   - Install the game to system"
	@echo "  uninstall - Remove the game from system"
	@echo "  reinstall - Uninstall, rebuild, and install again"

.PHONY: all build create_dirs clean rebuild install uninstall reinstall help