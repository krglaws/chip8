# set compiler
CC := gcc

# set compiler flags
CFLAGS := `sdl2-config --libs --cflags` --std=c99 -Wall -lSDL2_image -lm

# header files
HDRS := src/chip8core.h

# source files
SRCS := src/chip8.c

# names of outputted object files
OBJS := $(SRCS:.c=.o)

# name of final executable
EXEC := bin/chip8

# default recipe
all: $(EXEC)

# recipe for building the final executable
$(EXEC): $(OBJS) $(HDRS) Makefile
	$(CC) -o $@ $(OBJS) $(CFLAGS)

# recipe for building object files
#$(OBJS): $(@:.o=.c) $(HDRS) Makefile
#	$(CC) -o $@ $(@:.o=.c) -c $(CFLAGS)

# recipe to clean the workspace
clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: all clean
