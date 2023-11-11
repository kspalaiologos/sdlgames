
#ifndef _UI_H
#define _UI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define WIN_WIDTH 900
#define WIN_HEIGHT 700

#define FPS 24

#define CARD_WIDTH 71
#define CARD_HEIGHT 96

#define STACK_X_OFFSET 30
#define STACK_Y_OFFSET 50

#define HIDDEN_Y_OFFSET 8
#define VISIBLE_Y_OFFSET 30
#define STACK_X_SPACING 85

extern SDL_Window * window;
extern SDL_Renderer * renderer;

extern SDL_Texture * card_backs[13];
extern SDL_Texture * card_faces[4][13];

extern SDL_Texture *background, *spiderCard;

extern TTF_Font *font, *big_font;

void tick_fireworks(uint64_t ticks);
void render_fireworks();
void load_textures();
int create_window();
void render_felt();

void render_card(int x, int y, int suit, int rank);
void render_card_back(int x, int y);
void render_spider_card(int x, int y);
void render_hollow_card(int x, int y);

#endif
