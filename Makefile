CC = gcc
CFLAGS = -Wall -Wextra -MMD -MP -std=c23 -D_GNU_SOURCE
LIBS = -lncurses -lpthread -L/usr/lib -larena -lprocps

SRC_DIRS = src src/window src/monitor src/thread src/util src/colors src/task
OBJ_DIR = obj
RC_DIR = /usr/local/share/mtop
RC_FILES = colors

SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)
BINARY = mtop

all: $(BINARY)

install:
	make && cp mtop /usr/bin
	mkdir -p $(RC_DIR)
	cp $(RC_FILES) $(RC_DIR)


debug: CFLAGS += -g -DDEBUG
debug: $(BINARY)

$(BINARY): $(OBJECTS) 	
	$(CC) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) mtop
