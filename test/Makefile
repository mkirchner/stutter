CC=clang
CFLAGS=-g -Wall -Wextra -pedantic -I../include -I../lib/gc/src -fprofile-arcs -ftest-coverage
LDFLAGS=-g -L../build/src -L../lib/gc/src --coverage
LDLIBS=
RM=rm
BUILD_DIR=../build

TARGETS=$(BUILD_DIR)/test/test_list

.PHONY: all
all: $(TARGETS)
	$(BUILD_DIR)/test/test_list

#
# test_list
#
$(BUILD_DIR)/test/test_list:
	mkdir -p $(BUILD_DIR)/test
	mkdir -p $(BUILD_DIR)/lib/gc/src
	$(CC) $(CFLAGS) -MMD -c ../lib/gc/src/gc.c -o $(BUILD_DIR)/lib/gc/src/gc.o
	$(CC) $(CFLAGS) -MMD -c ../lib/gc/src/log.c -o $(BUILD_DIR)/lib/gc/src/log.o
	$(CC) $(CFLAGS) -MMD -c test_list.c -o $(BUILD_DIR)/test/test_list.o
	$(CC) $(LDFLAGS) $(LDLIBS) \
	       	$(BUILD_DIR)/lib/gc/src/gc.o \
		$(BUILD_DIR)/lib/gc/src/log.o \
		$(BUILD_DIR)/test/test_list.o -o $(BUILD_DIR)/test/test_list

