# Files to include
#C_SRC  = $(wildcard *.c)
#OBJECTS := $(patsubst %.c,%.o,$(C_SRC))
OBJECTS = \
		  app/test_midi.o \
		  core/block.o \
		  core/error.o \
		  core/note.o \
		  core/ntype.o \
		  blocks/audio/compressor.o \
		  blocks/audio/function_gen.o \
		  blocks/audio/impulse.o \
		  blocks/audio/lpf.o \
		  blocks/audio/mixer.o \
		  blocks/audio/recorder.o \
		  blocks/audio/sample.o \
		  blocks/audio/wave.o \
		  blocks/io/portaudio.o \
		  blocks/io/midi_reader.o \
		  blocks/io/midi_integrator.o \
		  blocks/accumulator.o \
		  blocks/constant.o \
		  blocks/debug.o \
		  blocks/fittings.o \
		  blocks/instruments/instrument.o \
		  blocks/instruments/sine.o \
		  blocks/instruments/saw.o \
		  blocks/maths.o \
		  blocks/sequencer.o \
		  blocks/synth.o \

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
override CFLAGS += $(INC) -O0 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused -Wwrite-strings -std=c99 
#override CFLAGS += $(INC) -O3 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused -Wwrite-strings -std=c99 
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

