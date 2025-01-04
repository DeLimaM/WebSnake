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
	rm -rf $(TEST_BUILD_DIR)

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

TEST_SRC_DIR = tests/src
TEST_BUILD_DIR = tests/build
TEST_OUT_DIR = tests/out
TEST_SOURCES = $(shell find $(TEST_SRC_DIR) -name '*.c')
TEST_TARGETS = $(TEST_SOURCES:$(TEST_SRC_DIR)/%.c=$(TEST_BUILD_DIR)/%)

tests: create_test_dirs $(TEST_TARGETS)
	@for test in $(TEST_TARGETS); do ./$$test; done

test-%: create_test_dirs $(TEST_BUILD_DIR)/%
	./$(TEST_BUILD_DIR)/$*

create_test_dirs:
	mkdir -p $(TEST_BUILD_DIR)
	mkdir -p $(TEST_OUT_DIR)

$(TEST_BUILD_DIR)/%: $(TEST_SRC_DIR)/%.c $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS))
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: all build clean rebuild create_dirs install uninstall reinstall \
        tests test-% create_test_dirs help