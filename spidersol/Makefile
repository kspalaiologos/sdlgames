
CC=emcc
CFLAGS=-O3 -flto -g3
LDFLAGS=-lSDL2 -lSDL2_ttf -lm

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

spidrsol: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o spidrsol
