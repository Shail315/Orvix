
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11

TARGET = orvix
BUILD_DIR = .build
PARSER = $(BUILD_DIR)/ORVIX.tab.c
LEXER = $(BUILD_DIR)/lex.yy.c
CPPFLAGS = -I..

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(PARSER): ../orvix.y ../src/ast.h ../src/runtime.h | $(BUILD_DIR)
	bison -d -Wall -o $(PARSER) ../orvix.y

$(LEXER): ../orvix.l $(PARSER)
	flex -o $(LEXER) ../orvix.l

$(TARGET): $(PARSER) $(LEXER) ../src/ast.c ../src/runtime.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $(TARGET) $(PARSER) $(LEXER) ../src/ast.c ../src/runtime.c -lfl

run: $(TARGET)
	./$(TARGET) sum.orx

clean:
	rm -rf $(TARGET) $(BUILD_DIR)

.PHONY: all run clean
