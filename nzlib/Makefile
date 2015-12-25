# Files to include
C_SRC = \
		std.c \
		types/std.c \
		types/midi.c \
		blocks/constant.c \
		blocks/debug.c \
		blocks/accumulator.c \
		blocks/fittings.c \
		blocks/maths.c \
		blocks/lpf.c \
		blocks/io/portaudio.c \
		blocks/io/midi_reader.c \
		blocks/io/midi_smf.c \
		blocks/io/midi_melody.c \
		blocks/io/midi_drums.c \
		blocks/io/wavfile.c \
		blocks/audio/envelope.c \
		blocks/audio/mixer.c \
		blocks/audio/wave.c

OBJECTS = $(C_SRC:%.c=%.o)

TARGET = nzstd.so

CC = gcc

INC = -I. -I..
LIB = -L/usr/local/lib -L..

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)

# compile and generate dependency info;
%.o: %.c
	gcc -c $(CFLAGS) $*.c -o $*.o
	gcc -MM $(CFLAGS) $*.c > $*.d

# Assembler, compiler, and linker flags
override CFLAGS += $(INC) -O0 -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused -Wwrite-strings -std=c99 -fPIC
override LDFLAGS += $(LIB) $(CFLAGS)
LIBS = -lnoise -lm -lportaudio -lsndfile -lpthread

# Targets
.PHONY: clean all
.DEFAULT_GOAL = all
all: $(TARGET) $(APP_TARGETS)
clean:
	-rm -f $(OBJECTS) $(TARGET) $(DEPS)

$(TARGET): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$@ $(LDFLAGS) -o $@ $^ $(LIBS)
