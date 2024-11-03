CC = gcc
CFLAGS = -Wall -Wextra -g -MMD -MP -D_XOPEN_SOURCE_EXTENDED
LIBS = -lncursesw -lpthread -L/usr/lib -larena

SRC_DIRS = src src/window src/monitor
OBJ_DIR = obj

SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)
BINARY = mtop

all: $(BINARY)

$(BINARY): $(OBJECTS) 	
	$(CC) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) mtop
