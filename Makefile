CC = gcc
CFLAGS = -Wall -Wextra -MMD -MP
LIBS = -lncurses -lpthread -L/usr/lib -larena

SRC_DIRS = src src/window src/monitor src/thread src/util
OBJ_DIR = obj

SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)
BINARY = mtop

all: $(BINARY)

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
