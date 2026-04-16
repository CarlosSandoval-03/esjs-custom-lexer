CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -Iinclude

SRC_DIR := src
BUILD_DIR := build/obj
TARGET := main

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

DOCS_DIR := docs

.PHONY: build run clean docs

# Objetivo por defecto
build: $(TARGET)

# Enlaza el ejecutable final en la raiz del proyecto
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Compila cada fuente en build/obj/*.o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Crea el directorio de build si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compila y ejecuta
run: build
	@./$(TARGET)

clean:
	rm -rf build $(TARGET)

docs:
	rm -rf $(DOCS_DIR)
	doxygen