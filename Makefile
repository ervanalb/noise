# Files to include
#C_SRC  = $(wildcard *.c)
#OBJECTS := $(patsubst %.c,%.o,$(C_SRC))
OBJECTS = \
		  accumulator.o \
		  block.o \
		  constant.o \
		  debug.o \
		  error.o \
		  fittings.o \
		  function_gen.o \
		  maths.o \
		  mixer.o \
		  sequencer.o \
		  soundcard.o \
		  test.o \
		  typefns.o \
		  wave.o

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

