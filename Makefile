#
# stutter Makefile
#
GIT_VERSION := "$(shell git describe --abbrev=8 --dirty --always --tags)"

CC=clang
CFLAGS=-g -Wall -Wextra -pedantic -Iinclude -Ilib/gc/src -D__STUTTER_VERSION__=\"$(GIT_VERSION)\"
LDFLAGS=-g -Lbuild/src -Lbuild/lib/gc/src
LDLIBS=-ledit
RM=rm
BUILD_DIR=./build

STUTTER_BINARY=stutter
STUTTER_SRCS=$(wildcard src/*.c) lib/gc/src/gc.c
STUTTER_OBJS=$(STUTTER_SRCS:%.c=$(BUILD_DIR)/%.o)

.PHONY: stutter
stutter: $(BUILD_DIR)/$(STUTTER_BINARY)

$(BUILD_DIR)/$(STUTTER_BINARY): $(STUTTER_OBJS)
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

$(BUILD_DIR)/src/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/lib/gc/src/%.o: lib/gc/src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

.PHONY: test
test:
	$(MAKE) -C $@

.PHONY: clean
clean:
	$(RM) -f $(STUTTER_OBJS)
	$(MAKE) -C test clean

distclean: clean
	$(RM) -f $(BUILD_DIR)/$(STUTTER_BINARY)
	$(RM) -f $(BUILD_DIR)/test/*gcd*
	$(MAKE) -C test distclean

