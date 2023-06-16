
# -- Compiler flags
CC=gcc
CFLAGS=-Os -flto -g3
LDFLAGS=-lSDL2 -lSDL2_ttf -lSDL2_mixer -lm

# -- Targets
.PHONY: all
all: assets sdlmine

.PHONY: clean
clean:
	@rm -f assets/*.h assets.h assets.c *.o sdlmine
	@touch assets.h assets.c

.PHONY: format
format:
	clang-format -i *.c *.h

# -- Generate assets.h
.PHONY: assets
assets:
	./gen_assets.sh

# -- Source code
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

sdlmine: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


