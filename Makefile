# Files to include
C_SRC = \
		util.c \
		error.c \
		context.c \
		ntype.c \
		block.c \
		graph.c \
		argparse.c

OBJECTS = $(C_SRC:%.c=src/%.o)

APP_SRC = \
		test_types.c \
		test_block.c \
		test_graph.c \
		test_argparse.c \
		test_midi_new.c

APP_TARGETS = $(APP_SRC:%.c=noise_%)
APP_OBJECTS = $(APP_SRC:%.c=app/%.o)

TARGET = libnoise.so

CC = gcc

INC = -I.
LIB = -L/usr/local/lib

DEPS = $(OBJECTS:.o=.d) $(APP_OBJECTS:.o=.d)
-include $(DEPS)

# compile and generate dependency info;
%.o: %.c
	gcc -c $(CFLAGS) $*.c -o $*.o
	gcc -MM $(CFLAGS) $*.c > $*.d

# Assembler, compiler, and linker flags
override CFLAGS += $(INC) -O0 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused -Wwrite-strings -std=c99 -fPIC
override LFLAGS += $(LIB) $(CFLAGS)
LIBS = -ldl
APP_LIBS = -lnoise nzmod/libstd.so

# Targets
.PHONY: clean all
.DEFAULT_GOAL = all
all: $(TARGET) $(APP_TARGETS)
clean:
	-rm -f $(OBJECTS) $(APP_OBJECTS) $(APP_TARGETS) $(DEPS)

$(TARGET): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$@ -o $@ $^ $(LIBS)

noise_%: app/%.o $(TARGET)
	$(CC) $(LFLAGS) -o $@ $^ -L. $(APP_LIBS)
