
#ifndef _GAME_RENDER_H_
#define _GAME_RENDER_H_

#include <SDL2/SDL.h>

void draw_bevel(SDL_Renderer * renderer, SDL_Rect rect, int thickness);
void render_3num(SDL_Renderer * renderer, int num, int x, int y);
void render_minefield(SDL_Renderer * renderer, int offX, int offY, int selCellX, int selCellY);
void render_face(SDL_Renderer * renderer, char grimace, int offsetX, int offsetY);

#endif
