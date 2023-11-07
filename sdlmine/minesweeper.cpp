
#include "main.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern "C" {
    #include "assets.h"
    #include "game_render.h"
    #include "logic.h"
}

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl2.h"
#include "../imgui/imgui_impl_sdlrenderer2.h"

SDL_Window * window;
SDL_Renderer * renderer;

bool black_white = false, sounds_enabled = true;
int state = STATE_GAME;

const int MENU_HEIGHT=20;

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

int mode = -1;
int xcopy = -1, ycopy = -1, mcopy = -1;

bool running = true;
SDL_Event event;
uint64_t frame_start;
int may_restart = 0, game_over = 0, game_start, counter2 = 0;

int timer_started = 0, mouth_idx = 0;

ImGuiIO io;

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

void restart() {
    regenerate_minefield(minefield_x, minefield_y, minefield_mines);
    calculate_window_dimensions();
    SDL_SetWindowSize(window, window_dim_x, window_dim_y);
    int rw = 0, rh = 0;
    SDL_GetRendererOutputSize(renderer, &rw, &rh);
    if(rw != window_dim_x) {
        float widthScale = (float)rw / (float) window_dim_x;
        float heightScale = (float)rh / (float) window_dim_y;

        if(widthScale != heightScale) {
            fprintf(stderr, "WARNING: width scale != height scale\n");
        }

        SDL_RenderSetScale(renderer, widthScale, heightScale);
    }
    timer_started = 0;
    may_restart = 0;
    game_over = 0;
    mouth_idx = 0;
    game_start = time(NULL);
}

void game_loop(void) {
    #ifdef EMSCRIPTEN
    if (!running) {
        emscripten_cancel_main_loop();
        return;
    }
    #endif
    while (SDL_PollEvent(&event)) {
        if (state == STATE_GAME) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (io.WantCaptureMouse) break;
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
                        } else if (!game_over && event.button.y > 15 + MENU_HEIGHT) {
                            // Set the mouth to the "o" shape
                            mouth_idx = 2;
                            break;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (io.WantCaptureMouse) break;
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        // If within the bounds of the face
                        SDL_Rect rect;
                        rect.x = (window_dim_x - 24) / 2;
                        rect.y = 15 + MENU_HEIGHT;
                        rect.w = 24;
                        rect.h = 24;
                        if (!game_over) mouth_idx = 0;
                        if (event.button.x >= rect.x && event.button.x < rect.x + rect.w &&
                            event.button.y >= rect.y && event.button.y < rect.y + rect.h) {
                            if (may_restart) {
                                restart();
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
                                if (timer_started == 0) {
                                    if (minefield[x + y * minefield_x] != 0) {
                                        safepoint_x = x; safepoint_y = y;
                                        regenerate_minefield(minefield_x, minefield_y, minefield_mines);
                                    }
                                    timer_started = 1;
                                }
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
            }
        } else if (state == STATE_SETTINGS) {
            switch(event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
            }
        }
        if (ImGui_ImplSDL2_ProcessEvent(&event)) continue;
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
        int cursor_y = (y - (67 + MENU_HEIGHT)) / 16;
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

        // TODO Render the menu and overlays here...
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Settings")) {
                if (ImGui::MenuItem("Sound", nullptr, sounds_enabled, true)) {
                    sounds_enabled = !sounds_enabled;
                }
                if (ImGui::MenuItem("Black & White", nullptr, black_white, true)) {
                    black_white = !black_white;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Mode")) {
                bool restarted = false;
                if (ImGui::MenuItem("Easy", nullptr, mode == 0, true)) {
                    minefield_x = 9; minefield_y = 9; minefield_mines = 10;
                    mode = 0; restarted = true;
                }
                if (ImGui::MenuItem("Medium", nullptr, mode == 1, true)) {
                    minefield_x = 16; minefield_y = 16; minefield_mines = 40;
                    mode = 1; restarted = true;
                }
                if (ImGui::MenuItem("Hard", nullptr, mode == 2, true)) {
                    minefield_x = 32; minefield_y = 16; minefield_mines = 99;
                    mode = 2; restarted = true;
                }
                if (ImGui::MenuItem("Custom", nullptr, mode == 3, true)) {
                    mode = 3; restarted = true;
                    xcopy = minefield_x; ycopy = minefield_y; mcopy = minefield_mines;
                    state = STATE_SETTINGS;
                }
                if(restarted) {
                    restart();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (state == STATE_SETTINGS) {
            ImGui::SetNextWindowSize(ImVec2(125, 180));
            // Slider for x between 9 and 32.
            ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            ImGui::Text("Width");
            ImGui::SliderInt("##x", &xcopy, 9, 32);
            ImGui::Text("Height");
            ImGui::SliderInt("##y", &ycopy, 9, 32);
            ImGui::Text("Mines");
            ImGui::SliderInt("##m", &mcopy, 10, 150);
            if (ImGui::Button("OK")) {
                if (minefield_mines > minefield_x * minefield_y - 10) {
                    mcopy = minefield_x * minefield_y - 10;
                }
                minefield_x = xcopy; minefield_y = ycopy; minefield_mines = mcopy;
                calculate_window_dimensions();
                SDL_SetWindowSize(window, window_dim_x, window_dim_y);
                int rw = 0, rh = 0;
                SDL_GetRendererOutputSize(renderer, &rw, &rh);
                if(rw != window_dim_x) {
                    float widthScale = (float)rw / (float) window_dim_x;
                    float heightScale = (float)rh / (float) window_dim_y;

                    if(widthScale != heightScale) {
                        fprintf(stderr, "WARNING: width scale != height scale\n");
                    }

                    SDL_RenderSetScale(renderer, widthScale, heightScale);
                }
                state = STATE_GAME;
                mode = 3;
                restart();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                state = STATE_GAME;
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

        SDL_RenderPresent(renderer);
        frame_start = SDL_GetTicks64();
    } else {
        SDL_Delay(1);
    }
}

int main(void) {
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
                              SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        return 1;
    }

    int rw = 0, rh = 0;
    SDL_GetRendererOutputSize(renderer, &rw, &rh);
    if(rw != window_dim_x) {
        float widthScale = (float)rw / (float) window_dim_x;
        float heightScale = (float)rh / (float) window_dim_y;

        if(widthScale != heightScale) {
            fprintf(stderr, "WARNING: width scale != height scale\n");
        }

        SDL_RenderSetScale(renderer, widthScale, heightScale);
    }

    assets_load(renderer);

    SDL_SetWindowIcon(window, mine);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = NULL;

    ImGui::StyleColorsClassic();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    if (minefield_x == 9 && minefield_y == 9 && minefield_mines == 10)
        mode = 0; // easy
    else if (minefield_x == 16 && minefield_y == 16 && minefield_mines == 40)
        mode = 1; // medium
    else if (minefield_x == 32 && minefield_y == 16 && minefield_mines == 99)
        mode = 2; // hard
    else
        mode = 3; // custom
    
    frame_start = SDL_GetTicks64();
    game_start = time(NULL);

    #ifndef EMSCRIPTEN
    while (running)
        game_loop();
    #else
    emscripten_set_main_loop(game_loop, 0, 1);
    #endif

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    assets_free(renderer);
    SDL_Quit();
}
