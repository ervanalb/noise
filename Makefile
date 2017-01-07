CC=gcc
CFLAGS=-I. -Ilibdill -g -O2 -std=gnu99 -Wall -Wextra -Werror
LDFLAGS=-ldill -lm -lsndfile -lportaudio -ldl

NZSRCS=$(wildcard blocks/*.c)
NZOBJECTS=$(NZSRCS:.c=.nzo)
OBJECTS=noise.o output.o main.o

%.o : %.c
	$(CC) $(CFLAGS) -rdynamic -c $< -o $@

noise : $(OBJECTS)
	$(CC) $^ $(CFLAGS) -rdynamic $(LDFLAGS) -o $@

%.nzo : %.c
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@

nzobjects : $(NZOBJECTS)

clean:
	rm -r $(OBJECTS) $(NZOBJECTS) noise

all: noise nzobjects

.PHONY: clean
.PHONY: nzobjects
.DEFAULT_GOAL := all
