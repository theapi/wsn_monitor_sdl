# Adapted from https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

# set the compiler
CC := gcc

TARGET_EXEC ?= monitor

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


$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(CFLAGS)

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
