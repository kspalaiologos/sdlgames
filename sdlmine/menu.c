
#include "menu.h"

#include <stdbool.h>

#include "assets.h"
#include "main.h"

#define MENU_PADDING 4

SDL_Rect menuButtons[2];
void (*menuActions[2])(void);
int highlightedMenu = -1;

void onclickSettings() { state = STATE_SETTINGS; }

void onclickQuit() { exit(0); }

void hover_menu(int x, int y) {
    highlightedMenu = -1;
    if (y == 0) return;
    for (int i = 0; i < 2; i++) {
        if (x >= menuButtons[i].x && x >= menuButtons[i].y && x <= menuButtons[i].x + menuButtons[i].w &&
            y <= menuButtons[i].y + menuButtons[i].h) {
            highlightedMenu = i;
            break;
        }
    }
}

void menu_click(int x, int y) {
    if (y == 0) return;
    if (highlightedMenu != -1) {
        int n = highlightedMenu;
        highlightedMenu = -1;
        menuActions[n]();
    }
}

void render_menu(SDL_Renderer * renderer, int width) {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = width;
    rect.h = MENU_HEIGHT;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
    rect.x++;
    rect.y++;
    rect.w -= 2;
    rect.h -= 2;
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &rect);

#define createmenu(var, selvar, name, action)                                 \
    {                                                                         \
        SDL_Color color = { 0, 0, 0, 255 };                                   \
        SDL_Surface * surface = TTF_RenderText_Blended(micross, name, color); \
        var = SDL_CreateTextureFromSurface(renderer, surface);                \
        SDL_Color selcolor = { 255, 255, 255, 255 };                          \
        surface = TTF_RenderText_Blended(micross, name, selcolor);            \
        selvar = SDL_CreateTextureFromSurface(renderer, surface);             \
        SDL_QueryTexture(var, NULL, NULL, &tmp_rect.w, &tmp_rect.h);          \
        menuButtons[current_item] = tmp_rect;                                 \
        menuActions[current_item] = action;                                   \
        current_item++;                                                       \
        tmp_rect.x += tmp_rect.w + 2 * MENU_PADDING;                          \
        SDL_FreeSurface(surface);                                             \
    }

    // Draw menu options:
    // New Game | Settings | Quit
    static SDL_Texture * newGameTexture = NULL;
    static SDL_Texture * settingsTexture = NULL;
    static SDL_Texture * selnewGameTexture = NULL;
    static SDL_Texture * selsettingsTexture = NULL;
    if (newGameTexture == NULL) {
        int current_item = 0;
        SDL_Rect tmp_rect;
        tmp_rect.x = 2 * MENU_PADDING;
        tmp_rect.y = MENU_PADDING;
        createmenu(settingsTexture, selsettingsTexture, "Options", onclickSettings);
        createmenu(newGameTexture, selnewGameTexture, "Quit", onclickQuit);
    }

#undef createmenu

#define rendermenu(name, selname)                                              \
    {                                                                          \
        if (n++ == highlightedMenu) {                                          \
            SDL_QueryTexture(selname, NULL, NULL, &menu_rect.w, &menu_rect.h); \
            SDL_Rect border;                                                   \
            border.x = menu_rect.x - MENU_PADDING;                             \
            border.y = 1;                                                      \
            border.h = MENU_HEIGHT - 2;                                        \
            border.w = menu_rect.w + 2 * MENU_PADDING;                         \
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);                 \
            SDL_RenderDrawRect(renderer, &border);                             \
            SDL_SetRenderDrawColor(renderer, 20, 20, 128, 255);                \
            rect.x++;                                                          \
            rect.y++;                                                          \
            rect.w -= 2;                                                       \
            rect.h -= 2;                                                       \
            SDL_RenderFillRect(renderer, &border);                             \
            SDL_RenderCopy(renderer, selname, NULL, &menu_rect);               \
        } else {                                                               \
            SDL_QueryTexture(name, NULL, NULL, &menu_rect.w, &menu_rect.h);    \
            SDL_RenderCopy(renderer, name, NULL, &menu_rect);                  \
        }                                                                      \
        menu_rect.x += menu_rect.w + 2 * MENU_PADDING;                         \
    }

    SDL_Rect menu_rect;
    int n = 0;
    menu_rect.x = 2 * MENU_PADDING;
    menu_rect.y = MENU_PADDING - 2;
    rendermenu(settingsTexture, selsettingsTexture);
    rendermenu(newGameTexture, selnewGameTexture);

#undef rendermenu
}
