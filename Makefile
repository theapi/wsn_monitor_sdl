# Adapted from https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

#PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

# set the compiler
CC := cc

TARGET_EXEC ?= monitor

BIN_DIR ?= bin
BUILD_DIR ?= build
SRC_DIRS ?= src
INC_DIRS ?= inc

SRCS := $(shell find $(SRC_DIRS) -name *.c )
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# set the compiler flags
CFLAGS := `sdl2-config --libs --cflags` -ggdb3 -O0 --std=c11 -Wall -lSDL2_image -lm -lSDL2_ttf

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@


$(BIN_DIR)/$(TARGET_EXEC): $(OBJS)
	$(MKDIR_P) $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(CFLAGS)

.PHONY: clean install uninstall

clean:
	$(RM) -r $(BUILD_DIR) $(BIN_DIR)

install:
	@echo "Installing to: "$(DESTDIR)$(PREFIX)/wsn_monitor
	mkdir -p $(DESTDIR)$(PREFIX)/wsn_monitor/bin
	cp bin/monitor $(DESTDIR)$(PREFIX)/wsn_monitor/bin/monitor
	cp -R resources $(DESTDIR)$(PREFIX)/wsn_monitor/resources

uninstall:
	@echo "Unstalling from: "$(DESTDIR)$(PREFIX)/wsn_monitor
	rm -fr $(DESTDIR)$(PREFIX)/wsn_monitor

-include $(DEPS)

MKDIR_P ?= mkdir -p