CC=clang
CFLAGS=-g -Wall -Wextra -pedantic -I../include -fprofile-arcs -ftest-coverage
LDFLAGS=-g -L../build/src --coverage
LDLIBS=-ledit
RM=rm
BUILD_DIR=../build/test

.PHONY: all
all: $(BUILD_DIR)/test_stutter \

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# note: ../src/gc.c is directly included in test_gc.c in order to gain
#       access to static functions
SRCS=test_stutter.c \
    ../src/array.c \
    ../src/ast.c \
    ../src/core.c \
    ../src/djb2.c \
    ../src/env.c \
    ../src/eval.c \
    ../src/ir.c \
    ../src/lexer.c \
    ../src/list.c \
    ../src/log.c \
    ../src/map.c \
    ../src/primes.c \
    ../src/reader.c \
    ../src/reader_stack.c \
    ../src/value.c

OBJS=$(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS=$(OBJS:%.o=%.d)

$(BUILD_DIR)/test_stutter: $(OBJS)
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

.PHONY: clean
clean:
	$(RM) -f $(OBJS) $(DEPS)

distclean: clean
	$(RM) -f $(BUILD_DIR)/test_stutter
	$(RM) -f $(BUILD_DIR)/test_stutter

