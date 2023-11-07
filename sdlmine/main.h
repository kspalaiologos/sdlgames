
#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef _cplusplus
extern "C" {
#endif

#define FPS 24

#include <SDL2/SDL.h>
#include <stdbool.h>

extern bool black_white, sounds_enabled;
extern SDL_Window * window;
extern SDL_Renderer * renderer;

extern int state;

enum { STATE_GAME, STATE_SETTINGS };

#ifdef _cplusplus
}
#endif

#endif
