#
# stutter Makefile
#

CC=clang
CFLAGS=-g -Wall -Wextra -pedantic
LDFLAGS=-g
LDLIBS=-ledit
RM=rm
BINARY=stutter
BUILD_DIR=./build
SRCS=prompt.c
OBJS=$(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS=$(OBJS:%.o=%.d)

all: $(BUILD_DIR)/$(BINARY)

$(BUILD_DIR)/$(BINARY): $(OBJS)
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

-include $(DEPS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

.PHONY: clean
clean:
	$(RM) -f $(OBJS) $(DEPS)

distclean: clean
	$(RM) -f $(BUILD_DIR)/$(BINARY)

.PHONY: test
test:
	$(MAKE) -C test
