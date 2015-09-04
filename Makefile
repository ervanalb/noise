# Files to include
#C_SRC  = $(wildcard *.c)
#OBJECTS := $(patsubst %.c,%.o,$(C_SRC))
OBJECTS = \
		  block.o \
		  blocks/accumulator.o \
		  blocks/constant.o \
		  blocks/debug.o \
		  blocks/fittings.o \
		  blocks/function_gen.o \
		  blocks/impulse.o \
		  blocks/lpf.o \
		  blocks/maths.o \
		  blocks/mixer.o \
		  blocks/recorder.o \
		  blocks/sample.o \
		  blocks/sequencer.o \
		  blocks/wave.o \
		  error.o \
		  test.o \
		  ntypes.o 

TARGET = noise

CC = gcc

INC = -I.
LIB = -L/usr/local/lib

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)

# compile and generate dependency info;
%.o: %.c
	gcc -c $(CFLAGS) $*.c -o $*.o
	gcc -MM $(CFLAGS) $*.c > $*.d

# Assembler, compiler, and linker flags
#override CFLAGS += $(INC) -O0 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused -Wwrite-strings -std=c99 
override CFLAGS += $(INC) -O3 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused -Wwrite-strings -std=c99 
override LFLAGS += $(LIB) $(CFLAGS)
LIBS = -lm -lportaudio -lsndfile

# Targets
.PHONY: clean all
.DEFAULT_GOAL = all
all: $(TARGET)
clean:
	-rm -f $(OBJECTS) $(OUTPUT) $(DEPS)

$(TARGET): $(OBJECTS)
	$(CC) $(LFLAGS) -o $@ $^ $(LIBS)

