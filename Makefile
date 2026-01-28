# NetLang Compiler Makefile
# Builds the lexer + parser into a complete compiler frontend
#
# Target: Intel Core i5-5200U (Broadwell) with AVX2/SSE4.2
# Author: Abhijeet Deb Nath

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g -O2
# AVX2/SSE optimizations for Broadwell (enable when doing codegen)
# CFLAGS += -march=broadwell -mavx2 -msse4.2

# Tools
FLEX = flex
BISON = bison

# Directories
SRC_DIR = src
BUILD_DIR = build
LEXER_DIR = $(SRC_DIR)/lexer
PARSER_DIR = $(SRC_DIR)/parser
AST_DIR = $(SRC_DIR)/ast

# Output
TARGET = $(BUILD_DIR)/netlang

# Generated files
LEX_C = $(BUILD_DIR)/lex.yy.c
BISON_C = $(BUILD_DIR)/net_lang.tab.c
BISON_H = $(BUILD_DIR)/net_lang.tab.h

# Source files
AST_SRC = $(AST_DIR)/ast.c

# Object files
OBJS = $(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/net_lang.tab.o $(BUILD_DIR)/ast.o

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Generate parser (Bison) - creates .tab.c and .tab.h
$(BISON_C) $(BISON_H): $(PARSER_DIR)/net_lang.y
	$(BISON) -d -v -o $(BISON_C) $<
	@echo "Bison: Generated parser"

# Generate lexer (Flex)
$(LEX_C): $(LEXER_DIR)/net_lang.l $(BISON_H)
	$(FLEX) -o $@ $<
	@echo "Flex: Generated lexer"

# Compile lexer
$(BUILD_DIR)/lex.yy.o: $(LEX_C)
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -I$(AST_DIR) -c -o $@ $<

# Compile parser
$(BUILD_DIR)/net_lang.tab.o: $(BISON_C)
	$(CC) $(CFLAGS) -I$(BUILD_DIR) -I$(AST_DIR) -c -o $@ $<

# Compile AST
$(BUILD_DIR)/ast.o: $(AST_SRC)
	$(CC) $(CFLAGS) -I$(AST_DIR) -c -o $@ $<

# Link everything
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo ""
	@echo "Build complete: $(TARGET)"

# Test with example files
test: $(TARGET)
	@echo ""
	@echo "========== Testing demo.nlang =========="
	./$(TARGET) examples/demo.nlang || ./$(TARGET) demo.nlang

# Run on inception demo
test-inception: $(TARGET)
	@echo ""
	@echo "========== Testing inception_demo.nlang =========="
	./$(TARGET) examples/inception_demo.nlang || ./$(TARGET) inception_demo.nlang

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory"

# Clean and rebuild
rebuild: clean all

# Install flex and bison (for reference)
deps:
	@echo "On Windows (MSYS2/MinGW):"
	@echo "  pacman -S flex bison"
	@echo ""
	@echo "On Ubuntu/Debian:"
	@echo "  sudo apt-get install flex bison"

# Show project structure
info:
	@echo "NetLang Compiler Structure:"
	@echo "  src/lexer/net_lang.l    - Flex lexer specification"
	@echo "  src/parser/net_lang.y   - Bison parser grammar"
	@echo "  src/ast/ast.h           - AST node definitions"
	@echo "  src/ast/ast.c           - AST implementation"
	@echo "  build/                  - Generated files"
	@echo ""
	@echo "Build targets:"
	@echo "  make          - Build compiler"
	@echo "  make test     - Test with demo.nlang"
	@echo "  make clean    - Remove build files"

.PHONY: all clean rebuild test test-inception deps info
