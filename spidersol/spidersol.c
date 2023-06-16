
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cntrl.h"
#include "deal.h"
#include "engine.h"
#include "game_state.h"
#include "infobox.h"
#include "leaderboard.h"
#include "settings.h"
#include "ui.h"
#include "undo.h"
#include "vector.h"
#include "win.h"

#define EXTRA_CARDS_X_OFFSET 740
#define EXTRA_CARDS_Y_OFFSET 570
#define EXTRA_CARDS_X_SPACING 10

#define KINGS_X_OFFSET 50
#define KINGS_Y_OFFSET 570
#define KINGS_X_SPACING 10

#define MENU_HEIGHT 25
#define MENU_PADDING 5

/* Misc game stuff */
SDL_Rect menuButtons[7];
void (*menuActions[7])(void);
int highlightedMenu = -1;

/* Dragging. */
int dragging_stack, dragging_stack_offset, dragging_stack_x, dragging_stack_y;
int dragging_mouse_off_x, dragging_mouse_off_y;

/* Main code. */
void render_stacks(void) {
    // First, render the extra deals (spider cards).
    for (int i = 0; i < game.remainingExtraDeals; i++) {
        render_spider_card(EXTRA_CARDS_X_OFFSET + i * EXTRA_CARDS_X_SPACING, EXTRA_CARDS_Y_OFFSET);
    }

    // Render the kings.
    for (int i = 0; i < 8; i++) {
        if (game.wonKings[i] == -1) break;
        render_card(KINGS_X_OFFSET + i * KINGS_X_SPACING, KINGS_Y_OFFSET, game.wonKings[i], 0);
    }

    // Simple path: No card is being dragged.
    if (game.state != STATE_GAME_DRAGGING_STACK) {
        for (int i = 0; i < 10; i++) {
            int rect_y = STACK_Y_OFFSET;

            if (game.stacks[i].num_cards == 0) {
                render_hollow_card(STACK_X_OFFSET + i * STACK_X_SPACING, STACK_Y_OFFSET);
                continue;
            }

            for (int j = 0; j < game.stacks[i].num_cards; j++) {
                // If j >= stacks[i].num_cards - stacks[visible_offset], this card is visible.
                // Otherwise, it's hidden.

                int x = STACK_X_OFFSET + i * STACK_X_SPACING;
                int y = rect_y;

                game.stacks[i].cards[j].x = x;
                game.stacks[i].cards[j].y = y;

                if (j >= game.stacks[i].num_cards - game.stacks[i].visible_offset) {
                    render_card(x, y, game.stacks[i].cards[j].suit, game.stacks[i].cards[j].value);
                    rect_y += VISIBLE_Y_OFFSET;
                } else {
                    render_card_back(x, y);
                    rect_y += HIDDEN_Y_OFFSET;
                }
            }
        }
    } else if (game.state == STATE_GAME_DRAGGING_STACK) {
        // A card is being dragged.
        // First, render all stacks besides the one we want right now.
        for (int i = 0; i < 10; i++) {
            if (i == dragging_stack) {
                continue;
            }

            if (game.stacks[i].num_cards == 0) {
                render_hollow_card(STACK_X_OFFSET + i * STACK_X_SPACING, STACK_Y_OFFSET);
                continue;
            }

            int rect_y = STACK_Y_OFFSET;
            for (int j = 0; j < game.stacks[i].num_cards; j++) {
                // If j >= stacks[i].num_cards - stacks[visible_offset], this card is visible.
                // Otherwise, it's hidden.

                int x = STACK_X_OFFSET + i * STACK_X_SPACING;
                int y = rect_y;

                game.stacks[i].cards[j].x = x;
                game.stacks[i].cards[j].y = y;

                if (j >= game.stacks[i].num_cards - game.stacks[i].visible_offset) {
                    render_card(x, y, game.stacks[i].cards[j].suit, game.stacks[i].cards[j].value);
                    rect_y += VISIBLE_Y_OFFSET;
                } else {
                    render_card_back(x, y);
                    rect_y += HIDDEN_Y_OFFSET;
                }
            }
        }

        // Render the remaining part of the stack we're dragging.
        int rect_y = STACK_Y_OFFSET;

        if (dragging_stack_offset == 0) {
            render_hollow_card(STACK_X_OFFSET + dragging_stack * STACK_X_SPACING, STACK_Y_OFFSET);
        }

        for (int j = 0; j < dragging_stack_offset; j++) {
            int x = STACK_X_OFFSET + dragging_stack * STACK_X_SPACING;
            int y = rect_y;

            game.stacks[dragging_stack].cards[j].x = x;
            game.stacks[dragging_stack].cards[j].y = y;

            if (j >= game.stacks[dragging_stack].num_cards - game.stacks[dragging_stack].visible_offset) {
                render_card(x, y, game.stacks[dragging_stack].cards[j].suit,
                            game.stacks[dragging_stack].cards[j].value);
                rect_y += VISIBLE_Y_OFFSET;
            } else {
                render_card_back(x, y);
                rect_y += HIDDEN_Y_OFFSET;
            }
        }

        // Render the dragged part.
        int base_x = dragging_stack_x - dragging_mouse_off_x, base_y = dragging_stack_y - dragging_mouse_off_y;
        for (int j = dragging_stack_offset; j < game.stacks[dragging_stack].num_cards; j++) {
            int x = base_x;
            int y = base_y + (j - dragging_stack_offset) * VISIBLE_Y_OFFSET;

            game.stacks[dragging_stack].cards[j].x = -1;
            game.stacks[dragging_stack].cards[j].y = -1;

            render_card(x, y, game.stacks[dragging_stack].cards[j].suit, game.stacks[dragging_stack].cards[j].value);
        }
    }
}

void drop_stack(int x, int y) {
    /* Try to drop the stack. Find on which card we were dropped. */
    int stack = -1, card = -1;
    for (int i = 0; i < 10; i++) {
        int j = game.stacks[i].num_cards - 1;
        if (j != -1) {
            if (x >= game.stacks[i].cards[j].x && x <= game.stacks[i].cards[j].x + CARD_WIDTH &&
                y >= game.stacks[i].cards[j].y && y <= game.stacks[i].cards[j].y + CARD_HEIGHT) {
                stack = i;
                card = j;
                break;
            }
        } else {
            // Dropping on an empty stack? Pretend there's a single card there:
            if (x >= STACK_X_OFFSET + i * STACK_X_SPACING && x <= STACK_X_OFFSET + i * STACK_X_SPACING + CARD_WIDTH &&
                y >= STACK_Y_OFFSET && y <= STACK_Y_OFFSET + CARD_HEIGHT) {
                stack = i;
                card = -1;
                break;
            }
        }
    }

    if (stack == -1 && card == -1) return;

    /* Check if the suit is the same and the value is one bigger. */
    if ((card != -1 &&
         game.stacks[stack].cards[card].suit == game.stacks[dragging_stack].cards[dragging_stack_offset].suit &&
         game.stacks[stack].cards[card].value == game.stacks[dragging_stack].cards[dragging_stack_offset].value - 1) ||
        card == -1) {
        /* Move the stack. */
        int moved_cards = game.stacks[dragging_stack].num_cards - dragging_stack_offset;
        for (int i = dragging_stack_offset; i < game.stacks[dragging_stack].num_cards; i++) {
            game.stacks[stack].cards[game.stacks[stack].num_cards] = game.stacks[dragging_stack].cards[i];
            game.stacks[stack].num_cards++;
            game.stacks[stack].visible_offset++;
        }

        game.stacks[dragging_stack].visible_offset -= moved_cards;
        game.stacks[dragging_stack].num_cards = dragging_stack_offset;
        if (game.stacks[dragging_stack].num_cards > 0 && game.stacks[dragging_stack].visible_offset <= 0)
            game.stacks[dragging_stack].visible_offset = 1;

        dragging_stack = -1;
        dragging_stack_offset = -1;
        dragging_stack_x = -1;
        dragging_stack_y = -1;

        game.moves++;
        game.points -= 4 - game.difficulty;
        game.needsTextRepaint = 1;

        try_collapse_row();
        sequencePoint();
        boardStateChangedDispatcher();
    }
}

void onclickSettings() {
    game.prevdiff = game.difficulty;
    game.state = STATE_GAME_SETTINGS;
}

void onclickQuit() { exit(0); }

void onclickLeaderboard() { game.state = STATE_GAME_LEADERBOARD; }

void render_menu() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = WIN_WIDTH;
    rect.h = MENU_HEIGHT;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
    rect.x++;
    rect.y++;
    rect.w -= 2;
    rect.h -= 2;
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &rect);

#define createmenu(var, selvar, name, action)                              \
    {                                                                      \
        SDL_Color color = { 0, 0, 0, 255 };                                \
        SDL_Surface * surface = TTF_RenderText_Blended(font, name, color); \
        var = SDL_CreateTextureFromSurface(renderer, surface);             \
        SDL_Color selcolor = { 255, 255, 255, 255 };                       \
        surface = TTF_RenderText_Blended(font, name, selcolor);            \
        selvar = SDL_CreateTextureFromSurface(renderer, surface);          \
        SDL_QueryTexture(var, NULL, NULL, &tmp_rect.w, &tmp_rect.h);       \
        menuButtons[current_item] = tmp_rect;                              \
        menuActions[current_item] = action;                                \
        current_item++;                                                    \
        tmp_rect.x += tmp_rect.w + 2 * MENU_PADDING;                       \
        SDL_FreeSurface(surface);                                          \
    }

    // Draw menu options:
    // New Game | Restart | Undo | Redo | Settings | Quit
    static SDL_Texture * newGameTexture = NULL;
    static SDL_Texture * restartTexture = NULL;
    static SDL_Texture * undoTexture = NULL;
    static SDL_Texture * redoTexture = NULL;
    static SDL_Texture * settingsTexture = NULL;
    static SDL_Texture * leaderboardTexture = NULL;
    static SDL_Texture * quitTexture = NULL;
    static SDL_Texture * selnewGameTexture = NULL;
    static SDL_Texture * selrestartTexture = NULL;
    static SDL_Texture * selundoTexture = NULL;
    static SDL_Texture * selredoTexture = NULL;
    static SDL_Texture * selsettingsTexture = NULL;
    static SDL_Texture * selleaderboardTexture = NULL;
    static SDL_Texture * selquitTexture = NULL;
    if (newGameTexture == NULL) {
        int current_item = 0;
        SDL_Rect tmp_rect;
        tmp_rect.x = 2 * MENU_PADDING;
        tmp_rect.y = MENU_PADDING;
        createmenu(newGameTexture, selnewGameTexture, "New Game", new_game);
        createmenu(restartTexture, selrestartTexture, "Restart", reset_game);
        createmenu(undoTexture, selundoTexture, "Undo", undo);
        createmenu(redoTexture, selredoTexture, "Redo", redo);
        createmenu(settingsTexture, selsettingsTexture, "Settings", onclickSettings);
        createmenu(leaderboardTexture, selleaderboardTexture, "Leaderboard", onclickLeaderboard);
        createmenu(quitTexture, selquitTexture, "Quit", onclickQuit);
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
    menu_rect.y = MENU_PADDING;
    rendermenu(newGameTexture, selnewGameTexture);
    rendermenu(restartTexture, selrestartTexture);
    rendermenu(undoTexture, selundoTexture);
    rendermenu(redoTexture, selredoTexture);
    rendermenu(settingsTexture, selsettingsTexture);
    rendermenu(leaderboardTexture, selleaderboardTexture);
    rendermenu(quitTexture, selquitTexture);

#undef rendermenu
}

#ifdef EMSCRIPTEN
    #include <emscripten.h>
#endif

int main(void) {
#ifdef EMSCRIPTEN
    EM_ASM(FS.mkdir('/offline'); FS.mount(IDBFS, {}, '/offline');

           FS.syncfs(true, function(err){ console.log("Error syncing data to disk.") }););
#endif
    if (TTF_Init() != 0) {
        fprintf(stderr, "Unable to initialize SDL_ttf: %s\n", TTF_GetError());
        return 1;
    }

    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    if (create_window()) return 1;

    load_textures();

    load_storage();
    atexit(save_storage);

    SDL_bool done = SDL_FALSE;
    uint64_t frame_start = SDL_GetTicks64();
    int secondsCurrent = time(NULL);
    while (!done) {
        if (time(NULL) != secondsCurrent &&
            (game.state == STATE_GAME_IDLE || game.state == STATE_GAME_DRAGGING_STACK)) {
            secondsCurrent = time(NULL);
            game.time++;
            game.needsTextRepaint = 1;
        }
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    done = SDL_TRUE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        switch (game.state) {
                            case STATE_GAME_IDLE:
                                for (int i = 0; i < 10; i++) {
                                    for (int j = game.stacks[i].num_cards - 1; j >= 0; j--) {
                                        if (j >= game.stacks[i].num_cards - game.stacks[i].visible_offset) {
                                            if (event.button.x >= game.stacks[i].cards[j].x &&
                                                event.button.x <= game.stacks[i].cards[j].x + CARD_WIDTH &&
                                                event.button.y >= game.stacks[i].cards[j].y &&
                                                event.button.y <= game.stacks[i].cards[j].y + CARD_HEIGHT) {
                                                // We've clicked on this card.
                                                // Before we start dragging, make sure that the color in the entire row
                                                // is the same and the values are ascending with difference of one.
                                                int color = game.stacks[i].cards[j].suit;
                                                int value = game.stacks[i].cards[j].value;
                                                int k;
                                                for (k = j; k < game.stacks[i].num_cards; k++) {
                                                    if (game.stacks[i].cards[k].suit != color ||
                                                        game.stacks[i].cards[k].value != value) {
                                                        break;
                                                    }
                                                    value++;
                                                }
                                                if (k != game.stacks[i].num_cards) {
                                                    break;
                                                }
                                                game.state = STATE_GAME_DRAGGING_STACK;
                                                dragging_stack = i;
                                                dragging_stack_offset = j;
                                                dragging_stack_x = event.button.x;
                                                dragging_stack_y = event.button.y;
                                                dragging_mouse_off_x = event.button.x - game.stacks[i].cards[j].x;
                                                dragging_mouse_off_y = event.button.y - game.stacks[i].cards[j].y;
                                                break;
                                            }
                                        }
                                    }
                                }
                                break;
                        }
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (game.state == STATE_GAME_DRAGGING_STACK) {
                        dragging_stack_x = event.motion.x;
                        dragging_stack_y = event.motion.y;
                    } else {
                        // Highlight menus?
                        highlightedMenu = -1;
                        for (int i = 0; i < 7; i++) {
                            if (event.motion.x >= menuButtons[i].x && event.motion.x >= menuButtons[i].y &&
                                event.motion.x <= menuButtons[i].x + menuButtons[i].w &&
                                event.motion.y <= menuButtons[i].y + menuButtons[i].h) {
                                highlightedMenu = i;
                                break;
                            }
                        }
                    }
                    break;
                case SDL_KEYDOWN:
                    cntrlHandleKey(event.key.keysym);
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        switch (game.state) {
                            case STATE_GAME_IDLE:
                                // Clicked to deal a new row?
                                if (game.remainingExtraDeals > 0 && event.button.x >= EXTRA_CARDS_X_OFFSET &&
                                    event.button.x <= EXTRA_CARDS_X_OFFSET + CARD_WIDTH * game.remainingExtraDeals &&
                                    event.button.y >= EXTRA_CARDS_Y_OFFSET &&
                                    event.button.y <= EXTRA_CARDS_Y_OFFSET + CARD_HEIGHT) {
                                    // Check if every slot has at least one card.
                                    int status = 1;
                                    for (int i = 0; i < 10; i++) {
                                        if (game.stacks[i].num_cards == 0) {
                                            status = 0;
                                            break;
                                        }
                                    }
                                    if (status) {
                                        game.state = STATE_GAME_DEALING_ROW;
                                        game.remainingExtraDeals--;
                                    }
                                } else if (highlightedMenu != -1) {
                                    (menuActions[highlightedMenu])();
                                }
                                break;
                            case STATE_GAME_FIREWORKS:
                                if (highlightedMenu != -1) {
                                    (menuActions[highlightedMenu])();
                                }
                                break;
                            case STATE_GAME_DRAGGING_STACK:
                                drop_stack(event.button.x, event.button.y);
                                game.state = STATE_GAME_IDLE;
                                break;
                            case STATE_GAME_SETTINGS:
                                settingsHandleMouseUp(event.button.x, event.button.y);
                                break;
                            case STATE_GAME_LEADERBOARD:
                                leaderboardHandleMouseUp(event.button.x, event.button.y);
                                break;
                            case STATE_GAME_LOST:
                                handleLossMouseUp(event.button.x, event.button.y);
                                if (highlightedMenu != -1) {
                                    (menuActions[highlightedMenu])();
                                }
                                break;
                        }
                    }
                    break;
            }
        }

        // Limit the game to 32fps.
        uint64_t frame_end = SDL_GetTicks64();
        if (frame_end - frame_start >= 1000 / FPS) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            render_felt();
            handleRenderInfobox();

            switch (game.state) {
                case STATE_GAME_IDLE:
                    if (game.wonKings[7] != -1) {
                        game.state = STATE_GAME_FIREWORKS;
                        handle_victory();
                    } else if (!has_moves()) {
                        game.state = STATE_GAME_LOST;
                    }
                    break;
                case STATE_GAME_DEAL:
                    deal();
                    SDL_Delay(10);
                    break;
                case STATE_GAME_DEALING_ROW:
                    deal_row();
                    SDL_Delay(40);
                    break;
            }

            render_stacks();
            cntrlRender();

            switch (game.state) {
                case STATE_GAME_FIREWORKS:
                    render_fireworks();
                    break;
                case STATE_GAME_SETTINGS:
                    settingsHandleRender();
                    break;
                case STATE_GAME_LEADERBOARD:
                    leaderboardHandleRender();
                    break;
                case STATE_GAME_LOST:
                    handle_loss();
                    break;
            }

            tick_fireworks(frame_end - frame_start);

            render_menu();

            SDL_RenderPresent(renderer);

            frame_start = SDL_GetTicks64();
        } else {
            SDL_Delay(1);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
