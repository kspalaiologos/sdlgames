
#ifndef _MENU_H_
#define _MENU_H_

#include <SDL2/SDL.h>

#define MENU_HEIGHT 20
void render_menu(SDL_Renderer * renderer, int width);
void hover_menu(int x, int y);
void menu_click(int x, int y);

#endif
