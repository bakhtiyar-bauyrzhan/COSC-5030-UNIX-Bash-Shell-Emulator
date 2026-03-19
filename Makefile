# Compiler and Flags
CC = gcc
CFLAGS = -Wall -g -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = build_obj
BIN_DIR = bin

# Output binary
TARGET = $(BIN_DIR)/myshell

# Files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default rule
all: directories $(TARGET)

# Create directories if they don't exist
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Linking
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the shell normally
run: all
	./$(TARGET)

# Run the shell with Valgrind to check for memory leaks
valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# Cleanup
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all directories clean run valgrind