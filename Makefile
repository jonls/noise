
RM = rm
CC = gcc -std=gnu99
CFLAGS = -Wall -O3 -march=core2 -pipe
LDFLAGS =

ifdef PROFILE
    CFLAGS += -g
    LDFLAGS += -pg
endif

noise: noise.c perlin.c simplex.c
	$(CC) $(CFLAGS) $(LDFLAGS) -lm `sdl-config --cflags --libs` -lGL -o $@ $^

.PHONY: clean
clean:
	$(RM) -f perlin simplex
