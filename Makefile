# Files to include
#C_SRC  = $(wildcard *.c)
#OBJECTS := $(patsubst %.c,%.o,$(C_SRC))
OBJECTS = \
		  accumulator.o \
		  block.o \
		  constant.o \
		  debug.o \
		  error.o \
		  maths.o \
		  test.o \
		  function_gen.o \
		  fittings.o \
		  typefns.o

TARGET = noise

CC = gcc

INC = -I
LIB = -L/usr/local/lib

# Assembler, compiler, and linker flags
override CFLAGS += $(INC) -O0 -g -Wall -Wextra -Werror -Wno-unused-parameter -std=c99
override LFLAGS += $(LIB) $(CFLAGS)
LIBS = -lm -lportaudio

# Targets
.PHONY: clean
all: $(TARGET)
clean:
	-rm -f *.o *.d $(OUTPUT)

$(TARGET): $(OBJECTS)
	$(CC) $(LFLAGS) -o $@ $^ $(LIBS)

