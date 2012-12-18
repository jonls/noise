
RM = rm
CC = gcc -std=gnu99
CFLAGS = -Wall -O3 -march=core2 -fwhole-program -pipe
LDFLAGS =

ifdef PROFILE
    CFLAGS += -g
    LDFLAGS += -pg
endif

all: perlin simplex

perlin: perlin.c
	$(CC) $(CFLAGS) $(LDFLAGS) -lm `sdl-config --cflags --libs` -lGL -o perlin perlin.c

simplex: simplex.c
	$(CC) $(CFLAGS) $(LDFLAGS) -lm `sdl-config --cflags --libs` -lGL -o simplex simplex.c

.PHONY: clean
clean:
	$(RM) -f perlin simplex
