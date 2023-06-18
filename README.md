
# SDL Games

A small collection of SDL games made under 24h. List of games:

- Spider Solitaire (Windows XP, 22-12-2022)
- Minesweeper (Windows XP, 11-06-2023)
- Reversi (Windows XP, 16-06-2023)

## Emscripten builds

Spider Solitaire:

```
emconfigure ./configure "CFLAGS=-sUSE_SDL=2 -sUSE_SDL_TTF=2 -std=gnu11 -sWASM=0 -O3
emmake make all
```

Reversi:

```
emconfigure ./configure "CFLAGS=-sUSE_SDL=2 -sUSE_SDL_TTF=2 -std=gnu11 -sWASM=0 -Os" \
                        "LDFLAGS=-sWASM=0 -sALLOW_MEMORY_GROWTH --memory-init-file 0 -sTOTAL_MEMORY=65536000" \
                        "CXXFLAGS=-sUSE_SDL=2 -sUSE_SDL_TTF=2 -std=gnu++17 -sWASM=0 -Os -sALLOW_MEMORY_GROWTH"
emmake make all
```
