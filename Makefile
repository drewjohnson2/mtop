UNAME_S := $(shell uname -s)
CC = gcc
CFLAGS = -Wall -Wextra -MMD -MP -std=c23 -D_GNU_SOURCE
SRC_DIRS = src src/window src/thread src/util src/colors src/task
OBJ_DIR = obj
RC_DIR = /usr/local/share/mtop
RC_FILES = colors

SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

ifeq ($(UNAME_S),Linux)
	LIBS = -lncurses -lpthread -L/usr/lib -larena -lprocps
	DEBUG_CFLAGS = -g -DDEBUG
	BIN_DIR = /usr/bin
	SOURCES += $(wildcard src/monitor/linux_*.c src/platform/linux_*.c)
else ifeq ($(UNAME_S),Darwin)
	LIBS = -lncurses -lpthread -L/usr/local/lib -larena
	DEBUG_CFLAGS = -g -DDEBUG
	BIN_DIR = /usr/local/bin
	SOURCES += $(wildcard src/monitor/apple_*.c src/platform/apple_*.c)
endif

OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)
BINARY = mtop

all: $(BINARY)

install:
	make 
ifeq ($(UNAME_S),Darwin)
	mkdir -p $(BIN_DIR)
endif
	cp mtop $(BIN_DIR)
	mkdir -p $(RC_DIR)
	cp $(RC_FILES) $(RC_DIR)

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(BINARY)

$(BINARY): $(OBJECTS) 	
	$(CC) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) mtop
