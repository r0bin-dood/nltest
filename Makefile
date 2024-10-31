TARGET_EXEC := nltest

SOURCE_DIR := .
INCLUDE_DIR := .

SOURCE_FILES := $(shell find $(SOURCE_DIR) -name '*.c')

INC_DIRS := $(shell find $(SOURCE_DIR) -type d -not -path '*/.*')
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CC := gcc
CFLAGS := -g3 -Wall
CPPFLAGS := $(INC_FLAGS) -I/usr/include -I/usr/include/libnl3/
LDFLAGS := -lnl-3 -lnl-genl-3

$(TARGET_EXEC): $(SOURCE_FILES)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)