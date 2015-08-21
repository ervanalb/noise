# Files to include
C_SRC  = $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(C_SRC))
OUTPUT = noise.so

CC = gcc

INC = -I
LIB = -L/usr/local/lib

# Assembler, compiler, and linker flags
override CFLAGS += $(INC) -O0 -g -Wall -Wextra -Werror -Wno-unused-parameter -fPIC -std=c99
override LFLAGS += $(LIB) $(CFLAGS) -shared
LIBS = -lm -lportaudio

# Targets
all: main
clean:
	-rm -f $(OBJECTS) $(OUTPUT)
main: $(OBJECTS)
	$(CC) $(LFLAGS) -o $(OUTPUT) $(OBJECTS) $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
