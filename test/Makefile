CC=clang
CFLAGS=-g -Wall -Wextra -pedantic -I../include -I../lib/gc/src -fprofile-arcs -ftest-coverage -Wno-gnu-zero-variadic-macro-arguments
LDFLAGS=-g -L../build/src -L../lib/gc/src --coverage
LDLIBS=
RM=rm
BUILD_DIR=../build

# targets are roughly in topological order
TARGETS=test_list \
	test_ast \
	test_array \
	test_djb2 \
	test_parser \
	test_primes \
	test_map \
	test_lexer \
	test_env \
	test_ir


define execute-command
$(1)

endef

.PHONY: all
all: $(TARGETS)
	$(foreach T,$(TARGETS),$(call execute-command,$(BUILD_DIR)/test/$(T)))
	$(BUILD_DIR)/stutter lang/core.stt
	$(BUILD_DIR)/stutter lang/more.stt

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/test/*
	rm -f $(BUILD_DIR)/lib/gc/src/*.o

test_setup:
	mkdir -p $(BUILD_DIR)/test
	mkdir -p $(BUILD_DIR)/lib/gc/src

gc: ../lib/gc/src/gc.c ../lib/gc/src/log.c
	$(CC) $(CFLAGS) -MMD -c ../lib/gc/src/gc.c -o $(BUILD_DIR)/lib/gc/src/gc.o
	$(CC) $(CFLAGS) -MMD -c ../lib/gc/src/log.c -o $(BUILD_DIR)/lib/gc/src/log.o

#
# test_list
#
test_list: test_setup gc
	$(CC) $(CFLAGS) -MMD -c test_list.c -o $(BUILD_DIR)/test/test_list.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
	       	$(BUILD_DIR)/lib/gc/src/gc.o \
		$(BUILD_DIR)/lib/gc/src/log.o \
	       	$(BUILD_DIR)/src/value.o \
		$(BUILD_DIR)/test/test_list.o -o $(BUILD_DIR)/test/test_list

#
# test_array
#
test_array: test_setup
	$(CC) $(CFLAGS) -MMD -c test_array.c -o $(BUILD_DIR)/test/test_array.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
		$(BUILD_DIR)/test/test_array.o -o $(BUILD_DIR)/test/test_array

#
# test_ast
#
test_ast: test_setup
	$(CC) $(CFLAGS) -MMD -c test_ast.c -o $(BUILD_DIR)/test/test_ast.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
		$(BUILD_DIR)/test/test_ast.o -o $(BUILD_DIR)/test/test_ast

#
# test_djb2
#
test_djb2: test_setup
	$(CC) $(CFLAGS) -MMD -c test_djb2.c -o $(BUILD_DIR)/test/test_djb2.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
		$(BUILD_DIR)/test/test_djb2.o -o $(BUILD_DIR)/test/test_djb2

#
# test_env
#
test_env: test_setup gc
	$(CC) $(CFLAGS) -MMD -c test_env.c -o $(BUILD_DIR)/test/test_env.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
		$(BUILD_DIR)/lib/gc/src/log.o \
	       	$(BUILD_DIR)/lib/gc/src/gc.o \
	       	$(BUILD_DIR)/src/djb2.o \
	       	$(BUILD_DIR)/src/list.o \
	       	$(BUILD_DIR)/src/map.o \
	       	$(BUILD_DIR)/src/primes.o \
	       	$(BUILD_DIR)/src/value.o \
		$(BUILD_DIR)/test/test_env.o -o $(BUILD_DIR)/test/test_env

#
# test_ir
#
test_ir: test_setup gc
	$(CC) $(CFLAGS) -MMD -c test_ir.c -o $(BUILD_DIR)/test/test_ir.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
		$(BUILD_DIR)/lib/gc/src/log.o \
	       	$(BUILD_DIR)/lib/gc/src/gc.o \
	       	$(BUILD_DIR)/src/ast.o \
	       	$(BUILD_DIR)/src/list.o \
	       	$(BUILD_DIR)/src/value.o \
		$(BUILD_DIR)/test/test_ir.o -o $(BUILD_DIR)/test/test_ir

#
# test_lexer
#
test_lexer: test_setup gc
	mkdir -p $(BUILD_DIR)/test/data
	$(CC) $(CFLAGS) -MMD -c test_lexer.c -o $(BUILD_DIR)/test/test_lexer.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
		$(BUILD_DIR)/test/test_lexer.o -o $(BUILD_DIR)/test/test_lexer

#
# test_map
#
test_map: test_setup gc
	$(CC) $(CFLAGS) -MMD -c test_map.c -o $(BUILD_DIR)/test/test_map.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
		$(BUILD_DIR)/lib/gc/src/log.o \
	       	$(BUILD_DIR)/lib/gc/src/gc.o \
	       	$(BUILD_DIR)/src/djb2.o \
	       	$(BUILD_DIR)/src/list.o \
	       	$(BUILD_DIR)/src/value.o \
	       	$(BUILD_DIR)/src/primes.o \
		$(BUILD_DIR)/test/test_map.o -o $(BUILD_DIR)/test/test_map

#
# test_parser
#
test_parser: test_setup gc
	mkdir -p $(BUILD_DIR)/test/data
	$(CC) $(CFLAGS) -MMD -c test_parser.c -o $(BUILD_DIR)/test/test_parser.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
		$(BUILD_DIR)/lib/gc/src/log.o \
	       	$(BUILD_DIR)/lib/gc/src/gc.o \
	       	$(BUILD_DIR)/src/lexer.o \
	       	$(BUILD_DIR)/src/list.o \
	       	$(BUILD_DIR)/src/value.o \
		$(BUILD_DIR)/test/test_parser.o -o $(BUILD_DIR)/test/test_parser

#
# test_primes
#
test_primes: test_setup gc
	mkdir -p $(BUILD_DIR)/test/dat
	$(CC) $(CFLAGS) -MMD -c test_primes.c -o $(BUILD_DIR)/test/test_primes.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
		$(BUILD_DIR)/test/test_primes.o -o $(BUILD_DIR)/test/test_primes

