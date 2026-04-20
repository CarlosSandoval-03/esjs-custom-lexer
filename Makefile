CC       := gcc
CFLAGS   := -Wall -Wextra -std=c11 -Iinclude

SRC_DIR   := src
BUILD_DIR := build/obj

# CLI executable
TARGET     := main
# Static library
LIB_TARGET := libesjs_lexer.a

# All source files
SRCS     := $(wildcard $(SRC_DIR)/*.c)
# Library sources — everything except the CLI driver
LIB_SRCS := $(filter-out $(SRC_DIR)/main.c,$(SRCS))

OBJS     := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
LIB_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

DOCS_DIR := docs

.PHONY: build lib run clean docs

# Default: build the CLI executable
build: $(TARGET)

# Build the static library only
lib: $(LIB_TARGET)

# ── Static library ────────────────────────────────────────────────────────────
$(LIB_TARGET): $(LIB_OBJS)
	ar rcs $@ $^

# ── CLI executable (links against library objects) ────────────────────────────
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# ── Object files ──────────────────────────────────────────────────────────────
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Crea el directorio de build si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compila y ejecuta
run: build
	@./$(TARGET)

clean:
	rm -rf build $(TARGET) $(LIB_TARGET)

docs:
	rm -rf $(DOCS_DIR)
	doxygen