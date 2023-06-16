
#include "main.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "assets.h"
#include "game_render.h"
#include "logic.h"
#include "menu.h"

SDL_Window * window;
SDL_Renderer * renderer;

bool black_white = false, sounds_enabled = true;
int state = STATE_GAME;

static unsigned window_dim_x = 0, window_dim_y = 0;
void calculate_window_dimensions() {
    // 16 x 16 is the size of a minefield cell.
    window_dim_x = 0;
    window_dim_y = 0;
    window_dim_x += 16 * minefield_x;
    window_dim_y += 16 * minefield_y;
    // 10px padding on both sides.
    window_dim_x += 20 + 6;
    window_dim_y += 30 + 34 + 6;
    // Menu:
    window_dim_y += MENU_HEIGHT;
}

int main(void) {
    int timer_started = 0, mouth_idx = 0;

    if (TTF_Init() != 0) {
        fprintf(stderr, "Unable to initialize SDL_ttf: %s\n", TTF_GetError());
        return 1;
    }

    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Unable to initialize SDL_mixer: %s\n", Mix_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    load_settings();
    atexit(save_settings);
    regenerate_minefield(minefield_x, minefield_y, minefield_mines);
    timer_started = 0;

    calculate_window_dimensions();
    window = SDL_CreateWindow("SDLMine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_dim_x, window_dim_y,
                              SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        return 1;
    }

    SDL_RenderSetLogicalSize(renderer, window_dim_x, window_dim_y);
    assets_load(renderer);

    SDL_SetWindowIcon(window, mine);

    // Some data for the settings box:
    SDL_Texture * txtMode = NULL;
    SDL_Texture * txtEasy = NULL;
    SDL_Texture * txtMedium = NULL;
    SDL_Texture * txtHard = NULL;
    SDL_Texture * txtCustom = NULL;
    SDL_Texture * txtSounds = NULL;
    SDL_Texture * txtBlackWhite = NULL;
    SDL_Texture * txtBounds = NULL;
    SDL_Texture * txtMines = NULL;
    SDL_Texture * seltxtEasy = NULL;
    SDL_Texture * seltxtMedium = NULL;
    SDL_Texture * seltxtHard = NULL;
    SDL_Texture * seltxtCustom = NULL;
    SDL_Texture * seltxtSounds = NULL;
    SDL_Texture * seltxtBlackWhite = NULL;

    SDL_Rect easyRect, mediumRect, hardRect, customRect, soundsRect, blackWhiteRect, boundsRect;
    SDL_Rect xdecRect, xincRect, ydecRect, yincRect, mdecRect, mincRect, totalRect;

    int mode = -1;
    if (minefield_x == 9 && minefield_y == 9 && minefield_mines == 10)
        mode = 0; // easy
    else if (minefield_x == 16 && minefield_y == 16 && minefield_mines == 40)
        mode = 1; // medium
    else if (minefield_x == 32 && minefield_y == 16 && minefield_mines == 99)
        mode = 2; // hard
    else
        mode = 3; // custom

    int xcopy = -1, ycopy = -1, mcopy = -1;

    SDL_bool running = SDL_TRUE;
    SDL_Event event;
    uint64_t frame_start = SDL_GetTicks64();
    int may_restart = 0, game_over = 0, game_start = time(NULL), counter2 = 0;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (state == STATE_GAME) {
                switch (event.type) {
                    case SDL_QUIT:
                        running = SDL_FALSE;
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            // If within the bounds of the face
                            SDL_Rect rect;
                            rect.x = (window_dim_x - 24) / 2;
                            rect.y = 15 + MENU_HEIGHT;
                            rect.w = 24;
                            rect.h = 24;
                            if (event.button.x >= rect.x && event.button.x < rect.x + rect.w &&
                                event.button.y >= rect.y && event.button.y < rect.y + rect.h) {
                                mouth_idx = 1;  // Pressed the mouth => restart the game.
                                may_restart = 1;
                                break;
                            } else if (!game_over && event.button.y > MENU_HEIGHT) {
                                // Set the mouth to the "o" shape
                                mouth_idx = 2;
                                break;
                            }
                        }
                        break;
                    case SDL_MOUSEBUTTONUP:
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            // If within the bounds of the face
                            SDL_Rect rect;
                            rect.x = (window_dim_x - 24) / 2;
                            rect.y = 15 + MENU_HEIGHT;
                            rect.w = 24;
                            rect.h = 24;
                            if (event.button.y <= MENU_HEIGHT) {
                                menu_click(event.button.x, event.button.y);
                                break;
                            }
                            if (!game_over) mouth_idx = 0;
                            if (event.button.x >= rect.x && event.button.x < rect.x + rect.w &&
                                event.button.y >= rect.y && event.button.y < rect.y + rect.h) {
                                if (may_restart) {
                                    regenerate_minefield(minefield_x, minefield_y, minefield_mines);
                                    calculate_window_dimensions();
                                    SDL_SetWindowSize(window, window_dim_x, window_dim_y);
                                    SDL_RenderSetLogicalSize(renderer, window_dim_x, window_dim_y);
                                    timer_started = 0;
                                    may_restart = 0;
                                    game_over = 0;
                                    mouth_idx = 0;
                                    game_start = time(NULL);
                                    break;
                                }
                            } else {
                                may_restart = 0;
                                if (game_over) mouth_idx = 3;
                            }

                            if (game_over) break;

                            rect.x = 13;
                            rect.y = 57 + MENU_HEIGHT;
                            rect.w = 16 * minefield_x;
                            rect.h = 16 * minefield_y;
                            if (event.button.x >= rect.x && event.button.x < rect.x + rect.w &&
                                event.button.y >= rect.y && event.button.y < rect.y + rect.h) {
                                int x = (event.button.x - rect.x) / 16;
                                int y = (event.button.y - rect.y) / 16;
                                if (x >= 0 && x < minefield_x && y >= 0 && y < minefield_y) {
                                    timer_started = 1;
                                    int r = make_move(x, y);
                                    if (r == -1) {
                                        mouth_idx = 3;
                                        game_over = 1;
                                        timer_started = 0;
                                        if (sounds_enabled) Mix_PlayChannel(-1, lose_snd, 0);
                                    } else if (r == 1) {
                                        mouth_idx = 4;
                                        game_over = 1;
                                        timer_started = 0;
                                        if (sounds_enabled) Mix_PlayChannel(-1, win_snd, 0);
                                    }
                                }
                            }
                        } else if (event.button.button == SDL_BUTTON_RIGHT && !game_over) {
                            // question marks/flags.
                            SDL_Rect rect;
                            rect.x = 13;
                            rect.y = 57 + MENU_HEIGHT;
                            rect.w = 16 * minefield_x;
                            rect.h = 16 * minefield_y;
                            if (event.button.x >= rect.x && event.button.x < rect.x + rect.w &&
                                event.button.y >= rect.y && event.button.y < rect.y + rect.h) {
                                int x = (event.button.x - rect.x) / 16;
                                int y = (event.button.y - rect.y) / 16;
                                if (x >= 0 && x < minefield_x && y >= 0 && y < minefield_y) {
                                    timer_started = 1;
                                    if(toggle_flag(x, y)) {
                                        mouth_idx = 4;
                                        game_over = 1;
                                        timer_started = 0;
                                        if (sounds_enabled) Mix_PlayChannel(-1, win_snd, 0);
                                    }
                                }
                            }
                            break;
                        }
                        break;
                    case SDL_MOUSEMOTION:
                        hover_menu(event.motion.x, event.motion.y);
                        // More events: ...
                }
            } else if (state == STATE_SETTINGS) {
                switch(event.type) {
                    case SDL_QUIT:
                        running = SDL_FALSE;
                        break;
                    case SDL_MOUSEBUTTONUP:
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            // If outside totalRect then return to STATE_GAME.
                            if (event.button.x < totalRect.x || event.button.x > totalRect.x + totalRect.w ||
                                event.button.y < totalRect.y || event.button.y > totalRect.y + totalRect.h) {
                                state = STATE_GAME;
                                minefield_x = xcopy;
                                minefield_y = ycopy;
                                minefield_mines = mcopy;
                                xcopy = -1;
                                ycopy = -1;
                                mcopy = -1;
                                regenerate_minefield(minefield_x, minefield_y, minefield_mines);
                                calculate_window_dimensions();
                                SDL_SetWindowSize(window, window_dim_x, window_dim_y);
                                timer_started = 0;
                                may_restart = 0;
                                game_over = 0;
                                mouth_idx = 0;
                                game_start = time(NULL);
                                break;
                            }
                            // If inside the bounds of easyRect:
                            if (event.button.x >= easyRect.x && event.button.x < easyRect.x + easyRect.w &&
                                event.button.y >= easyRect.y && event.button.y < easyRect.y + easyRect.h) {
                                xcopy = 9;
                                ycopy = 9;
                                mcopy = 10;
                                mode = 0;
                                break;
                            }
                            // If inside the bounds of mediumRect:
                            if (event.button.x >= mediumRect.x && event.button.x < mediumRect.x + mediumRect.w &&
                                event.button.y >= mediumRect.y && event.button.y < mediumRect.y + mediumRect.h) {
                                xcopy = 16;
                                ycopy = 16;
                                mcopy = 40;
                                mode = 1;
                                break;
                            }
                            // If inside the bounds of hardRect:
                            if (event.button.x >= hardRect.x && event.button.x < hardRect.x + hardRect.w &&
                                event.button.y >= hardRect.y && event.button.y < hardRect.y + hardRect.h) {
                                xcopy = 30;
                                ycopy = 16;
                                mcopy = 99;
                                mode = 2;
                                break;
                            }
                            // If inside the bounds of customRect:
                            if (event.button.x >= customRect.x && event.button.x < customRect.x + customRect.w &&
                                event.button.y >= customRect.y && event.button.y < customRect.y + customRect.h) {
                                mode = 3;
                                break;
                            }
                            // If inside the bounds of soundsRect:
                            if (event.button.x >= soundsRect.x && event.button.x < soundsRect.x + soundsRect.w &&
                                event.button.y >= soundsRect.y && event.button.y < soundsRect.y + soundsRect.h) {
                                sounds_enabled = !sounds_enabled;
                                break;
                            }
                            // If inside the bounds of blackWhiteRect:
                            if (event.button.x >= blackWhiteRect.x && event.button.x < blackWhiteRect.x + blackWhiteRect.w &&
                                event.button.y >= blackWhiteRect.y && event.button.y < blackWhiteRect.y + blackWhiteRect.h) {
                                black_white = !black_white;
                                break;
                            }
                            // If inside of xdecRect:
                            if (event.button.x >= xdecRect.x && event.button.x < xdecRect.x + xdecRect.w &&
                                event.button.y >= xdecRect.y && event.button.y < xdecRect.y + xdecRect.h) {
                                if (xcopy > 9) xcopy--;
                                break;
                            }
                            // If inside of xincRect:
                            if (event.button.x >= xincRect.x && event.button.x < xincRect.x + xincRect.w &&
                                event.button.y >= xincRect.y && event.button.y < xincRect.y + xincRect.h) {
                                if (xcopy < 32) xcopy++;
                                break;
                            }
                            // If inside of ydecRect:
                            if (event.button.x >= ydecRect.x && event.button.x < ydecRect.x + ydecRect.w &&
                                event.button.y >= ydecRect.y && event.button.y < ydecRect.y + ydecRect.h) {
                                if (ycopy > 9) ycopy--;
                                break;
                            }
                            // If inside of yincRect:
                            if (event.button.x >= yincRect.x && event.button.x < yincRect.x + yincRect.w &&
                                event.button.y >= yincRect.y && event.button.y < yincRect.y + yincRect.h) {
                                if (ycopy < 32) ycopy++;
                                break;
                            }
                            // If inside of mdecRect:
                            if (event.button.x >= mdecRect.x && event.button.x < mdecRect.x + mdecRect.w &&
                                event.button.y >= mdecRect.y && event.button.y < mdecRect.y + mdecRect.h) {
                                if (mcopy > 10) mcopy--;
                                break;
                            }
                            // If inside of mincRect:
                            if (event.button.x >= mincRect.x && event.button.x < mincRect.x + mincRect.w &&
                                event.button.y >= mincRect.y && event.button.y < mincRect.y + mincRect.h) {
                                if (mcopy < xcopy * ycopy - 20 && mcopy < 99) mcopy++;
                                break;
                            }
                        }
                }
            }
        }

        uint64_t frame_end = SDL_GetTicks64();
        if (frame_end - frame_start >= 1000 / FPS) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // Render the main game area.
            SDL_Rect rect;
            rect.x = 0;
            rect.y = 0;
            rect.w = window_dim_x;
            rect.h = window_dim_y;
            SDL_SetRenderDrawColor(renderer, 0xAF, 0xAF, 0xAF, 255);
            SDL_RenderFillRect(renderer, &rect);
            rect.x = 10;
            rect.y = 10 + MENU_HEIGHT;
            rect.w = window_dim_x - 20;
            rect.h = 34;
            draw_bevel(renderer, rect, 2);

            rect.x = 10;
            rect.y = 54 + MENU_HEIGHT;
            rect.w = window_dim_x - 20;
            rect.h = window_dim_y - (64 + MENU_HEIGHT);
            draw_bevel(renderer, rect, 3);

            int x, y;
            SDL_GetMouseState(&x, &y);
            int cursor_x = (x - 13) / 16;
            int cursor_y = (y - (57 + MENU_HEIGHT)) / 16;
            if (cursor_x < 0 || cursor_x > minefield_x || mouth_idx != 2 || state != STATE_GAME) cursor_x = -1;
            if (cursor_y < 0 || cursor_y > minefield_y || mouth_idx != 2 || state != STATE_GAME) cursor_y = -1;
            render_minefield(renderer, 13, 57 + MENU_HEIGHT, cursor_x, cursor_y);

            // Render the top bar.
            render_face(renderer, mouth_idx, (window_dim_x - 24) / 2, 15 + MENU_HEIGHT);

            if (timer_started) {
                int old = counter2;
                counter2 = time(NULL) - game_start;
                if (old != counter2 && sounds_enabled) Mix_PlayChannel(-1, tick_snd, 0);
            } else
                game_start = time(NULL);
            if (counter2 > 999) counter2 = 999;

            render_3num(renderer, mine_counter, 15, 15 + MENU_HEIGHT);
            render_3num(renderer, counter2, window_dim_x - 15 - 13 * 3, 15 + MENU_HEIGHT);

            render_menu(renderer, window_dim_x);

            if (state == STATE_SETTINGS) {
                if(xcopy == -1) {
                    xcopy = minefield_x;
                    ycopy = minefield_y;
                    mcopy = minefield_mines;
                }

                SDL_Rect rect;
                rect.x = (window_dim_x - 135) / 2;
                rect.y = (window_dim_y - 140) / 2;
                rect.w = 135;
                rect.h = 140;
                SDL_RenderFillRect(renderer, &rect);
                draw_bevel(renderer, rect, 3);
                totalRect = rect;

                if (txtMode == NULL) {
                    SDL_Color color = { 0, 0, 0, 255 };
                    SDL_Surface * surface;
                    
                    // Unselected.
                    surface = TTF_RenderText_Blended(micross, "Mode:", color);
                    txtMode = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);

                    surface = TTF_RenderText_Blended(micross, "Easy", color);
                    txtEasy = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);
                    
                    surface = TTF_RenderText_Blended(micross, "Medium", color);
                    txtMedium = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);

                    surface = TTF_RenderText_Blended(micross, "Hard", color);
                    txtHard = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);
                    
                    surface = TTF_RenderText_Blended(micross, "Custom", color);
                    txtCustom = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);

                    surface = TTF_RenderText_Blended(micross, "Sounds", color);
                    txtSounds = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);

                    surface = TTF_RenderText_Blended(micross, "B&W", color);
                    txtBlackWhite = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);

                    surface = TTF_RenderText_Blended(micross, "<     > x <     >", color);
                    txtBounds = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);

                    surface = TTF_RenderText_Blended(micross, "Mines: <     >", color);
                    txtMines = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);

                    color = (SDL_Color) { 255, 255, 255, 255 };

                    // Selected.
                    surface = TTF_RenderText_Blended(micross, "Easy", color);
                    seltxtEasy = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);
                    
                    surface = TTF_RenderText_Blended(micross, "Medium", color);
                    seltxtMedium = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);

                    surface = TTF_RenderText_Blended(micross, "Hard", color);
                    seltxtHard = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);
                    
                    surface = TTF_RenderText_Blended(micross, "Custom", color);
                    seltxtCustom = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);

                    surface = TTF_RenderText_Blended(micross, "Sounds", color);
                    seltxtSounds = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);

                    surface = TTF_RenderText_Blended(micross, "B&W", color);
                    seltxtBlackWhite = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);
                }

                SDL_SetRenderDrawColor(renderer, 20, 20, 128, 255);
                SDL_Rect text_rect;

                text_rect.x = rect.x + 10;
                text_rect.y = rect.y + 10;

                SDL_QueryTexture(txtMode, NULL, NULL, &text_rect.w, &text_rect.h);
                SDL_RenderCopy(renderer, txtMode, NULL, &text_rect);

                text_rect.x = rect.x + 15;
                text_rect.y += text_rect.h + 4;

                SDL_QueryTexture(txtEasy, NULL, NULL, &text_rect.w, &text_rect.h);
                if (mode == 0) {
                    text_rect.x -= 2; text_rect.y -= 2; text_rect.w += 4; text_rect.h += 4;
                    SDL_RenderFillRect(renderer, &text_rect);
                    text_rect.x += 2; text_rect.y += 2; text_rect.w -= 4; text_rect.h -= 4;
                    SDL_RenderCopy(renderer, seltxtEasy, NULL, &text_rect);
                } else SDL_RenderCopy(renderer, txtEasy, NULL, &text_rect);
                easyRect = text_rect;

                text_rect.x = rect.x + 15;
                text_rect.y += text_rect.h + 4;
                SDL_QueryTexture(txtMedium, NULL, NULL, &text_rect.w, &text_rect.h);
                if (mode == 1) {
                    text_rect.x -= 2; text_rect.y -= 2; text_rect.w += 4; text_rect.h += 4;
                    SDL_RenderFillRect(renderer, &text_rect);
                    text_rect.x += 2; text_rect.y += 2; text_rect.w -= 4; text_rect.h -= 4;
                    SDL_RenderCopy(renderer, seltxtMedium, NULL, &text_rect);
                } else SDL_RenderCopy(renderer, txtMedium, NULL, &text_rect);
                mediumRect = text_rect;

                text_rect.x = rect.x + 15;
                text_rect.y += text_rect.h + 4;
                SDL_QueryTexture(txtHard, NULL, NULL, &text_rect.w, &text_rect.h);
                if (mode == 2) {
                    text_rect.x -= 2; text_rect.y -= 2; text_rect.w += 4; text_rect.h += 4;
                    SDL_RenderFillRect(renderer, &text_rect);
                    text_rect.x += 2; text_rect.y += 2; text_rect.w -= 4; text_rect.h -= 4;
                    SDL_RenderCopy(renderer, seltxtHard, NULL, &text_rect);
                } else SDL_RenderCopy(renderer, txtHard, NULL, &text_rect);
                hardRect = text_rect;

                text_rect.x = rect.x + 15;
                text_rect.y += text_rect.h + 4;
                SDL_QueryTexture(txtCustom, NULL, NULL, &text_rect.w, &text_rect.h);
                if (mode == 3) {
                    text_rect.x -= 2; text_rect.y -= 2; text_rect.w += 4; text_rect.h += 4;
                    SDL_RenderFillRect(renderer, &text_rect);
                    text_rect.x += 2; text_rect.y += 2; text_rect.w -= 4; text_rect.h -= 4;
                    SDL_RenderCopy(renderer, seltxtCustom, NULL, &text_rect);
                } else SDL_RenderCopy(renderer, txtCustom, NULL, &text_rect);
                customRect = text_rect;

                text_rect.x = rect.x + 20;
                text_rect.y += text_rect.h + 4;
                SDL_QueryTexture(txtBounds, NULL, NULL, &text_rect.w, &text_rect.h);
                SDL_RenderCopy(renderer, txtBounds, NULL, &text_rect);

                text_rect.x = rect.x + 20;
                text_rect.y += text_rect.h + 4;
                SDL_QueryTexture(txtMines, NULL, NULL, &text_rect.w, &text_rect.h);
                SDL_RenderCopy(renderer, txtMines, NULL, &text_rect);

                text_rect.x = rect.x + 80;
                text_rect.y = rect.y + 10;
                SDL_QueryTexture(txtSounds, NULL, NULL, &text_rect.w, &text_rect.h);
                if (sounds_enabled) {
                    text_rect.x -= 2; text_rect.y -= 2; text_rect.w += 4; text_rect.h += 4;
                    SDL_RenderFillRect(renderer, &text_rect);
                    text_rect.x += 2; text_rect.y += 2; text_rect.w -= 4; text_rect.h -= 4;
                    SDL_RenderCopy(renderer, seltxtSounds, NULL, &text_rect);
                } else SDL_RenderCopy(renderer, txtSounds, NULL, &text_rect);
                soundsRect = text_rect;

                text_rect.x = rect.x + 80;
                text_rect.y += text_rect.h + 4;
                SDL_QueryTexture(txtBlackWhite, NULL, NULL, &text_rect.w, &text_rect.h);
                if (black_white) {
                    text_rect.x -= 2; text_rect.y -= 2; text_rect.w += 4; text_rect.h += 4;
                    SDL_RenderFillRect(renderer, &text_rect);
                    text_rect.x += 2; text_rect.y += 2; text_rect.w -= 4; text_rect.h -= 4;
                    SDL_RenderCopy(renderer, seltxtBlackWhite, NULL, &text_rect);
                } else SDL_RenderCopy(renderer, txtBlackWhite, NULL, &text_rect);
                blackWhiteRect = text_rect;

                // Render the bounds and # of mines.
                char dim1[10];

                text_rect.x = rect.x + 28;
                text_rect.y = rect.y + 100;
                SDL_Color color = { 0, 0, 0, 255 };
                SDL_Surface * surface;
                sprintf(dim1, "%02d", xcopy);
                surface = TTF_RenderText_Blended(micross, dim1, color);
                SDL_Texture * txtDim1 = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                SDL_QueryTexture(txtDim1, NULL, NULL, &text_rect.w, &text_rect.h);
                SDL_RenderCopy(renderer, txtDim1, NULL, &text_rect);
                SDL_DestroyTexture(txtDim1);
                xdecRect.x = text_rect.x - 10;
                xdecRect.y = text_rect.y;
                xdecRect.w = 10;
                xdecRect.h = text_rect.h;

                xincRect.x = text_rect.x + 10;
                xincRect.y = text_rect.y;
                xincRect.w = 10;
                xincRect.h = text_rect.h;

                text_rect.x = rect.x + 69;
                text_rect.y = rect.y + 100;
                sprintf(dim1, "%02d", ycopy);
                surface = TTF_RenderText_Blended(micross, dim1, color);
                txtDim1 = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                SDL_QueryTexture(txtDim1, NULL, NULL, &text_rect.w, &text_rect.h);
                SDL_RenderCopy(renderer, txtDim1, NULL, &text_rect);
                SDL_DestroyTexture(txtDim1);
                ydecRect.x = text_rect.x - 10;
                ydecRect.y = text_rect.y;
                ydecRect.w = 10;
                ydecRect.h = text_rect.h;

                yincRect.x = text_rect.x + 10;
                yincRect.y = text_rect.y;
                yincRect.w = 10;
                yincRect.h = text_rect.h;

                text_rect.x = rect.x + 66;
                text_rect.y = rect.y + 118;
                sprintf(dim1, "%02d", mcopy);
                surface = TTF_RenderText_Blended(micross, dim1, color);
                txtDim1 = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                SDL_QueryTexture(txtDim1, NULL, NULL, &text_rect.w, &text_rect.h);
                SDL_RenderCopy(renderer, txtDim1, NULL, &text_rect);
                SDL_DestroyTexture(txtDim1);
                mdecRect.x = text_rect.x - 10;
                mdecRect.y = text_rect.y;
                mdecRect.w = 10;
                mdecRect.h = text_rect.h;

                mincRect.x = text_rect.x + 10;
                mincRect.y = text_rect.y;
                mincRect.w = 10;
                mincRect.h = text_rect.h;
            }

            SDL_RenderPresent(renderer);
            frame_start = SDL_GetTicks64();
        } else {
            SDL_Delay(1);
        }
    }

    // Destroy all cached textures.
    SDL_DestroyTexture(txtMode);
    SDL_DestroyTexture(txtEasy);
    SDL_DestroyTexture(txtMedium);
    SDL_DestroyTexture(txtHard);
    SDL_DestroyTexture(txtCustom);
    SDL_DestroyTexture(txtSounds);
    SDL_DestroyTexture(txtBlackWhite);
    SDL_DestroyTexture(seltxtEasy);
    SDL_DestroyTexture(seltxtMedium);
    SDL_DestroyTexture(seltxtHard);
    SDL_DestroyTexture(seltxtCustom);
    SDL_DestroyTexture(seltxtSounds);
    SDL_DestroyTexture(seltxtBlackWhite);
    SDL_DestroyTexture(txtBounds);
    SDL_DestroyTexture(txtMines);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    assets_free(renderer);
    SDL_Quit();
}
