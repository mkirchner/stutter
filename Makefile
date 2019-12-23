#
# stutter Makefile
#

CC=clang
CFLAGS=-g -Wall -Wextra -pedantic -Iinclude -fprofile-arcs -ftest-coverage
LDFLAGS=-g -Lbuild/src --coverage
LDLIBS=-ledit
RM=rm
BUILD_DIR=./build

STUTTER_BINARY=stutter
STUTTER_SRCS=$(wildcard src/*.c)
STUTTER_OBJS=$(STUTTER_SRCS:%.c=$(BUILD_DIR)/%.o)
STUTTER_DEPS=$(STUTTER_OBJS:%.o=%.d)

.PHONY: stutter
stutter: $(BUILD_DIR)/$(STUTTER_BINARY)

$(BUILD_DIR)/$(STUTTER_BINARY): $(STUTTER_OBJS)
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

-include $(STUTTER_DEPS)

$(BUILD_DIR)/src/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

.PHONY: test
test:
	$(MAKE) -C $@
	$(BUILD_DIR)/test/test_stutter

.PHONY: clean
clean:
	$(RM) -f $(STUTTER_OBJS) $(STUTTER_DEPS)
	$(MAKE) -C test clean

distclean: clean
	$(RM) -f $(BUILD_DIR)/$(STUTTER_BINARY)
	$(RM) -f $(BUILD_DIR)/test/*gcd*
	$(MAKE) -C test distclean

