
# SDL Games

A small collection of SDL games made under 24h. List of games:

- Spider Solitaire (Windows XP, 22-12-2022)
- Minesweeper (Windows XP, 11-06-2023)
- Reversi (Windows XP, 16-06-2023)

## Emscripten builds

```
emconfigure ./configure "CFLAGS=-sUSE_SDL=2 -sUSE_SDL_TTF=2 -std=gnu11 -sWASM=0 -O3
emmake make all
```
