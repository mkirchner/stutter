#
# stutter Makefile
#

CC=clang
CFLAGS=-g -Wall -Wextra -pedantic -I./include
LDFLAGS=-g -L./build/src
LDLIBS=-ledit
RM=rm
BUILD_DIR=./build

BINARY=stutter
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS=$(OBJS:%.o=%.d)

TEST_BINARY=test_stutter
TEST_SRCS=$(wildcard test/*.c)
TEST_OBJS=$(filter-out ./build/src/main.o, $(OBJS)) $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o)
TEST_DEPS=$(TEST_OBJS:%.o=%.d)


.PHONY: all
all: $(BUILD_DIR)/$(BINARY)

$(BUILD_DIR)/$(BINARY): $(OBJS)
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

-include $(DEPS)

$(BUILD_DIR)/src/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/test/%.o: test/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

.PHONY: test
test: $(BUILD_DIR)/$(TEST_BINARY)
	$(BUILD_DIR)/$(TEST_BINARY)

$(BUILD_DIR)/$(TEST_BINARY): $(TEST_OBJS)
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

.PHONY: clean
clean:
	$(RM) -f $(OBJS) $(DEPS)
	$(RM) -f $(TEST_OBJS) $(TEST_DEPS)

distclean: clean
	$(RM) -f $(BUILD_DIR)/$(BINARY)
	$(RM) -f $(BUILD_DIR)/$(TEST_BINARY)

