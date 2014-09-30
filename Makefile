# Files to include
C_SRC  = $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(C_SRC))
OUTPUT = noise

CC = gcc

INC =
LIB = 

# Assembler, compiler, and linker flags
override CFLAGS += $(INC) -O0 -g -Wall
override LFLAGS += $(LIB) $(CFLAGS)
LIBS = -lm

# Targets
all: main
clean:
	-rm -f $(OBJECTS) $(OUTPUT)
main: $(OBJECTS)
	$(CC) $(LFLAGS) -o $(OUTPUT) $(OBJECTS) $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
