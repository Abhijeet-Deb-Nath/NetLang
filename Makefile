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
SEMANTIC_DIR = $(SRC_DIR)/semantic
CODEGEN_DIR = $(SRC_DIR)/codegen
GRAPH_DIR = $(SRC_DIR)/graph
PLANNER_DIR = $(SRC_DIR)/planner

# Output
TARGET = $(BUILD_DIR)/netlang

# Generated files
LEX_C = $(BUILD_DIR)/lex.yy.c
BISON_C = $(BUILD_DIR)/net_lang.tab.c
BISON_H = $(BUILD_DIR)/net_lang.tab.h

# Source files
AST_SRC = $(AST_DIR)/ast.c
SEMANTIC_SRC = $(SEMANTIC_DIR)/semantic.c $(SEMANTIC_DIR)/symbol_table.c $(SEMANTIC_DIR)/type_checker.c
GRAPH_SRC = $(GRAPH_DIR)/graph.c
PLANNER_SRC = $(PLANNER_DIR)/layout_plan.c $(PLANNER_DIR)/weight_pack_plan.c $(PLANNER_DIR)/memory_plan.c
CODEGEN_SRC = $(CODEGEN_DIR)/codegen.c $(CODEGEN_DIR)/conv_execution_plan.c $(CODEGEN_DIR)/kernel_selection.c $(CODEGEN_DIR)/fusion_optimizer.c
MAIN_SRC = $(SRC_DIR)/main.c

# Object files
OBJS = $(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/net_lang.tab.o $(BUILD_DIR)/ast.o \
       $(BUILD_DIR)/semantic.o $(BUILD_DIR)/symbol_table.o $(BUILD_DIR)/type_checker.o \
       $(BUILD_DIR)/graph.o $(BUILD_DIR)/layout_plan.o $(BUILD_DIR)/weight_pack_plan.o $(BUILD_DIR)/memory_plan.o \
       $(BUILD_DIR)/codegen.o $(BUILD_DIR)/conv_execution_plan.o $(BUILD_DIR)/kernel_selection.o $(BUILD_DIR)/fusion_optimizer.o $(BUILD_DIR)/main.o

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

# Compile semantic analyzer
$(BUILD_DIR)/semantic.o: $(SEMANTIC_DIR)/semantic.c
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -c -o $@ $<

$(BUILD_DIR)/symbol_table.o: $(SEMANTIC_DIR)/symbol_table.c
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -c -o $@ $<

$(BUILD_DIR)/type_checker.o: $(SEMANTIC_DIR)/type_checker.c
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -c -o $@ $<

# Compile graph IR
$(BUILD_DIR)/graph.o: $(GRAPH_SRC)
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -I$(GRAPH_DIR) -c -o $@ $<

# Compile layout planner
$(BUILD_DIR)/layout_plan.o: $(PLANNER_DIR)/layout_plan.c
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -I$(GRAPH_DIR) -I$(PLANNER_DIR) -c -o $@ $<

# Compile weight pack planner
$(BUILD_DIR)/weight_pack_plan.o: $(PLANNER_DIR)/weight_pack_plan.c
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -I$(GRAPH_DIR) -I$(PLANNER_DIR) -c -o $@ $<

# Compile memory planner
$(BUILD_DIR)/memory_plan.o: $(PLANNER_DIR)/memory_plan.c
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -I$(GRAPH_DIR) -I$(PLANNER_DIR) -c -o $@ $<

# Compile code generator
$(BUILD_DIR)/codegen.o: $(CODEGEN_DIR)/codegen.c
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -I$(GRAPH_DIR) -I$(PLANNER_DIR) -I$(CODEGEN_DIR) -c -o $@ $<

# Compile kernel selection policy
$(BUILD_DIR)/conv_execution_plan.o: $(CODEGEN_DIR)/conv_execution_plan.c
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -I$(GRAPH_DIR) -I$(PLANNER_DIR) -I$(CODEGEN_DIR) -c -o $@ $<

# Compile kernel selection policy
$(BUILD_DIR)/kernel_selection.o: $(CODEGEN_DIR)/kernel_selection.c
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -I$(GRAPH_DIR) -I$(PLANNER_DIR) -I$(CODEGEN_DIR) -c -o $@ $<

# Compile fusion optimizer
$(BUILD_DIR)/fusion_optimizer.o: $(CODEGEN_DIR)/fusion_optimizer.c
	$(CC) $(CFLAGS) -I$(AST_DIR) -I$(SEMANTIC_DIR) -I$(CODEGEN_DIR) -c -o $@ $<

# Compile main driver
$(BUILD_DIR)/main.o: $(MAIN_SRC)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c -o $@ $<

# Link everything
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo ""
	@echo "Build complete: $(TARGET)"

# Test with reference example
test: $(TARGET)
	@echo ""
	@echo "========== Testing lenet5.nlang =========="
	./$(TARGET) examples/lenet5.nlang

# Run on legacy inception fixture
test-inception: $(TARGET)
	@echo ""
	@echo "========== Testing inception.nlang =========="
	./$(TARGET) fixtures/legacy/inception.nlang

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
	@echo "  make test     - Test with lenet5.nlang"
	@echo "  make clean    - Remove build files"

.PHONY: all clean rebuild test test-inception deps info
