#!/bin/bash

rm -f assets.h assets.cpp assets_load.c assets_free.c assets_decl.c
touch assets.h assets.cpp assets_load.c assets_free.c assets_decl.c

echo "#ifndef ASSETS_H" >> assets.h
echo "#define ASSETS_H" >> assets.h
echo "" >> assets.h
echo "#include <SDL2/SDL.h>" >> assets.h
echo "#include <SDL2/SDL_mixer.h>" >> assets.h
echo "#include <SDL2/SDL_ttf.h>" >> assets.h
echo "" >> assets.h
echo "void assets_load(SDL_Renderer * renderer);" >> assets.h
echo "void assets_free();" >> assets.h
echo "" >> assets.h

cat > assets_load.c <<LOAD_EOF
#define loadbmp(buf) \
    { \
        SDL_Surface * surface = SDL_LoadBMP_RW(SDL_RWFromConstMem(assets_ ## buf ## _bmp, sizeof(assets_ ## buf ## _bmp)), 1); \
        if (surface == NULL) { \
            abort(); \
        } \
        SDL_SetColorKey(surface, SDL_TRUE, 0xFF00FF); \
        buf = SDL_CreateTextureFromSurface(renderer, surface); \
        SDL_FreeSurface(surface); \
    }
#define loadico(buf) \
    { \
        buf = SDL_LoadBMP_RW(SDL_RWFromConstMem(assets_ ## buf ## _ico, sizeof(assets_ ## buf ## _ico)), 1); \
        if (buf == NULL) { \
            abort(); \
        } \
    }
#define loadwav(buf) \
    { \
        buf = Mix_LoadWAV_RW(SDL_RWFromConstMem(assets_ ## buf ## _wav, sizeof(assets_ ## buf ## _wav)), 1); \
        if (buf == NULL) { \
            abort(); \
        } \
    }
#define loadttf(buf) \
    { \
        buf = TTF_OpenFontRW(SDL_RWFromConstMem(assets_ ## buf ## _ttf, sizeof(assets_ ## buf ## _ttf)), 1, 12); \
        if (buf == NULL) { \
            abort(); \
        } \
    }
LOAD_EOF
cat > assets_free.c <<FREE_EOF
#define freebmp(buf) \
    { \
        SDL_DestroyTexture(buf); \
    }
#define freewav(buf) \
    { \
        Mix_FreeChunk(buf); \
    }
#define freeico(buf) \
    { \
        SDL_FreeSurface(buf); \
    }
#define freettf(buf) \
    { \
        TTF_CloseFont(buf); \
    }
FREE_EOF

echo "void assets_load(SDL_Renderer * renderer) {" >> assets_load.c
echo "void assets_free() {" >> assets_free.c

# For each argument:
for f in assets/*; do
    # Get the extension; if the extension is .bmp set type to "SDL_Texture *", if the extension is .wav set type to "Mix_Chunk *"
    ext="${f##*.}"
    if [ "$ext" == "bmp" ]; then
        type="SDL_Texture *"
    elif [ "$ext" == "wav" ]; then
        type="Mix_Chunk *"
    elif [ "$ext" == "ico" ]; then
        type="SDL_Surface *"
    elif [ "$ext" == "ttf" ]; then
        type="TTF_Font *"
    else
        echo "Error: Invalid file type $ext, skipping..."
        continue
    fi

    # Get the file name without the extension or the path.
    name_noext="${f%.*}"
    name_noext="${name_noext##*/}"

    echo "extern $type $name_noext;" >> assets.h
    echo "$type $name_noext;" >> assets_decl.c
    xxd -i $f >> assets_decl.c
    echo "    load$ext($name_noext);" >> assets_load.c
    echo "    free$ext($name_noext);" >> assets_free.c
done

echo "}" >> assets_load.c
echo "}" >> assets_free.c

echo "#include \"assets.h\"" >> assets.cpp
echo "" >> assets.cpp
cat assets_decl.c >> assets.cpp
echo "" >> assets.cpp
cat assets_load.c >> assets.cpp
echo "" >> assets.cpp
cat assets_free.c >> assets.cpp
echo "" >> assets.cpp
echo "#endif" >> assets.h

rm -f assets_decl.c assets_load.c assets_free.c
