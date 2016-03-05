CC=gcc
CFLAGS=-I. -Ilibdill -g -O2 -std=gnu99 -Wall -Wextra -Werror
LDFLAGS=-ldill -lm -lsndfile

OBJECTS=noise.o main.o

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

noise : $(OBJECTS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

.PHONY: clean
.DEFAULT: all

clean:
	rm -r $(OBJECTS) noise

all: noise

