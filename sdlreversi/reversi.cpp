
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>

#include <iostream>
#include <mutex>
#include <thread>

#include "assets.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"

// Board
#define B_EMPTY 0
#define B_RED 1
#define B_BLUE 2
#define B_FLIP0 3
#define B_FLIP1 4
#define B_FLIP2 5
int board[8][8] = { B_EMPTY };

// Animation delay
int anim[8][8] = { 0 };

// AI level
int ai_level = 0;
bool animating = 0;
int player = B_RED;

// Engine
void new_game() {
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++) board[i][j] = B_EMPTY, anim[i][j] = 0;
    board[3][3] = B_RED;
    board[4][4] = B_RED;
    board[3][4] = B_BLUE;
    board[4][3] = B_BLUE;
}

bool is_move_possible(int x, int y) {
    if (animating && player == B_RED) return false;
    if (board[x][y] != B_EMPTY) return false;
    int dx, dy;
    for (dx = -1; dx <= 1; dx++)
        for (dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int i = x + dx, j = y + dy;
            if (i < 0 || i >= 8 || j < 0 || j >= 8) continue;
            if (board[i][j] != B_BLUE) continue;
            while (true) {
                i += dx;
                j += dy;
                if (i < 0 || i >= 8 || j < 0 || j >= 8) break;
                if (board[i][j] == B_EMPTY) break;
                if (board[i][j] == B_RED) return true;
            }
        }
    return false;
}

void make_player_move(int x, int y) {
    board[x][y] = B_RED;
    animating = true;
    int dx, dy;
    for (dx = -1; dx <= 1; dx++)
        for (dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int i = x + dx, j = y + dy;
            if (i < 0 || i >= 8 || j < 0 || j >= 8) continue;
            if (board[i][j] != B_BLUE) continue;
            while (true) {
                i += dx;
                j += dy;
                if (i < 0 || i >= 8 || j < 0 || j >= 8) break;
                if (board[i][j] == B_EMPTY) break;
                if (board[i][j] == B_RED) {
                    while (true) {
                        i -= dx;
                        j -= dy;
                        if (i == x && j == y) break;
                        anim[i][j] = 1;
                    }
                    break;
                }
            }
        }
    player = B_BLUE;
}

bool game_over() {
    // True if neither player can make a move.
    if (animating) return false;
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++) {
            if (board[y][x] != B_EMPTY) continue;

            // Check for red moves.
            int dx, dy;
            for (dx = -1; dx <= 1; dx++)
                for (dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    int i = x + dx, j = y + dy;
                    if (i < 0 || i >= 8 || j < 0 || j >= 8) continue;
                    if (board[i][j] != B_BLUE) continue;
                    while (true) {
                        i += dx;
                        j += dy;
                        if (i < 0 || i >= 8 || j < 0 || j >= 8) break;
                        if (board[i][j] == B_EMPTY) break;
                        if (board[i][j] == B_RED) return false;
                    }
                }

            // Check for blue moves.
            for (dx = -1; dx <= 1; dx++)
                for (dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    int i = x + dx, j = y + dy;
                    if (i < 0 || i >= 8 || j < 0 || j >= 8) continue;
                    if (board[i][j] != B_RED) continue;
                    while (true) {
                        i += dx;
                        j += dy;
                        if (i < 0 || i >= 8 || j < 0 || j >= 8) break;
                        if (board[i][j] == B_EMPTY) break;
                        if (board[i][j] == B_BLUE) return false;
                    }
                }
        }
    return true;
}

std::mutex ai_initializing;

// AI
extern "C" int init_ai();
extern "C" int ai_js(int * arr_board, int level, int ai_player);

#define MAX(a, b) ((a) > (b) ? (a) : (b))
void make_ai_move() {
    // The godawful AI code is going to spew stupid messages onto stdout/stderr,
    // temporarily prevent that:
    std::cout.setstate(std::ios::failbit);
    std::cerr.setstate(std::ios::failbit);
    ai_initializing.lock();
    int arr_board[64];
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++) arr_board[i * 8 + j] = board[i][j] == B_EMPTY ? -1 : board[i][j] == B_RED ? 1 : 0;
    int move = ai_js(arr_board, MAX(ai_level, 1) * 5, 0) / 1000;
    int x = move / 8, y = move % 8;
    board[x][y] = B_BLUE;
    anim[x][y] = -1;
    animating = true;
    int dx, dy;
    for (dx = -1; dx <= 1; dx++)
        for (dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int i = x + dx, j = y + dy;
            if (i < 0 || i >= 8 || j < 0 || j >= 8) continue;
            if (board[i][j] != B_RED) continue;
            while (true) {
                i += dx;
                j += dy;
                if (i < 0 || i >= 8 || j < 0 || j >= 8) break;
                if (board[i][j] == B_EMPTY) break;
                if (board[i][j] == B_BLUE) {
                    while (true) {
                        i -= dx;
                        j -= dy;
                        if (i == x && j == y) break;
                        anim[i][j] = -10;
                    }
                    break;
                }
            }
        }
    player = B_RED;
    ai_initializing.unlock();
    std::cout.clear();
    std::cerr.clear();
}

Uint32 animate(Uint32 interval, void * param) {
    if (animating) {
        bool made_changes = false;
        int i, j;
        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++)
                if (anim[i][j] > 0) {
                    anim[i][j]++;
                    if (anim[i][j] == 6) {
                        board[i][j] = B_FLIP0;
                        made_changes = true;
                    } else if (anim[i][j] == 11) {
                        board[i][j] = B_FLIP1;
                        made_changes = true;
                    } else if (anim[i][j] == 16) {
                        board[i][j] = B_FLIP2;
                        made_changes = true;
                    } else if (anim[i][j] == 21) {
                        board[i][j] = B_RED;
                        made_changes = true;
                    } else if (anim[i][j] == 26) {
                        anim[i][j] = 0;
                        made_changes = true;
                    } else {
                        anim[i][j]++;
                        made_changes = true;
                    }
                }
        if (made_changes) return interval;
        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++)
                if (anim[i][j] < 0 && anim[i][j] > -9) {
                    anim[i][j]--;
                    board[i][j] = abs(anim[i][j]) % 2 ? B_BLUE : B_EMPTY;
                    made_changes = true;
                } else if (anim[i][j] == -9) {
                    anim[i][j] = 0;
                    made_changes = true;
                }
        if (made_changes) return interval;
        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++)
                if (anim[i][j] <= -10) {
                    anim[i][j]--;
                    if (anim[i][j] == -16) {
                        board[i][j] = B_FLIP2;
                        made_changes = true;
                    } else if (anim[i][j] == -21) {
                        board[i][j] = B_FLIP1;
                        made_changes = true;
                    } else if (anim[i][j] == -26) {
                        board[i][j] = B_FLIP0;
                        made_changes = true;
                    } else if (anim[i][j] == -31) {
                        board[i][j] = B_BLUE;
                        made_changes = true;
                    } else if (anim[i][j] == -36) {
                        anim[i][j] = 0;
                        made_changes = true;
                    } else {
                        anim[i][j]--;
                        made_changes = true;
                    }
                }
        if (!made_changes) animating = false;
        return interval;
    }
    return interval;
}

#ifdef EMSCRIPTEN
    #include <emscripten.h>
    #include <emscripten/html5.h>
#endif

SDL_Window * window;
SDL_Renderer * renderer;
ImGuiIO io;
char difficulty;
bool done = false;

void game_loop() {
    #ifdef EMSCRIPTEN
    if(done) emscripten_cancel_main_loop();
    #endif

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEBUTTONUP:
                if (io.WantCaptureMouse) break;
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mx = event.button.x;
                    int my = event.button.y;
                    SDL_Rect r;
                    r.x = 35;
                    r.y = 66;
                    r.w = 27;
                    r.h = 27;
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            if (board[i][j] == B_EMPTY) {
                                if (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h &&
                                    is_move_possible(i, j)) {
                                    // OK, make move.
                                    make_player_move(i, j);
                                }
                            }
                            r.x += r.w + 4;
                        }
                        r.y += r.h + 4;
                        r.x = 35;
                    }
                }
                break;
        }
        if (ImGui_ImplSDL2_ProcessEvent(&event)) continue;
        if (event.type == SDL_QUIT ||
            (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window)))
            done = true;
    }

    if (!game_over() && !animating && player == B_BLUE) make_ai_move();

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    // Create a global menubar.
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Game")) {
            if (ImGui::MenuItem("New Game", "Ctrl+N")) {
                ai_level = difficulty;
                animating = false;
                new_game();
            }
            if (ImGui::MenuItem("Pass", "Ctrl+P")) {
                bool can_pass = true;
                for (int i = 0; i < 8 && can_pass; i++)
                    for (int j = 0; j < 8 && can_pass; j++)
                        if (board[i][j] == B_EMPTY && is_move_possible(i, j)) can_pass = false;
                if (can_pass) {
                    player = B_BLUE;
                }
            }
            if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
                done = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("AI")) {
            if (ImGui::MenuItem("Easy", nullptr, difficulty == 0, true)) {
                difficulty = 0;
            }
            if (ImGui::MenuItem("Medium", nullptr, difficulty == 1, true)) {
                difficulty = 1;
            }
            if (ImGui::MenuItem("Hard", nullptr, difficulty == 2, true)) {
                difficulty = 2;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (!animating && game_over()) {
        int blue_score = 0, red_score = 0;
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++) {
                if (board[i][j] == B_BLUE)
                    blue_score++;
                else if (board[i][j] == B_RED)
                    red_score++;
            }
        // "Game over" window.
        ImGui::SetNextWindowSize(ImVec2(180, 90));
        ImGui::Begin("Game over", nullptr, ImGuiWindowFlags_NoCollapse);
        if (blue_score > red_score)
            ImGui::Text("Blue wins!");
        else if (red_score > blue_score)
            ImGui::Text("Red wins!");
        else
            ImGui::Text("Draw!");
        ImGui::Text("%d red - %d blue", red_score, blue_score);
        if (ImGui::Button("New Game")) {
            ai_level = difficulty;
            animating = false;
            new_game();
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit")) {
            done = true;
        }
        ImGui::End();
    }

    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Paint the game field.
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 18;
    SDL_QueryTexture(orig, NULL, NULL, &rect.w, &rect.h);
    SDL_RenderCopy(renderer, orig, NULL, &rect);

    SDL_Rect r;
    r.x = 35;
    r.y = 66;
    r.w = 27;
    r.h = 27;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            switch (board[i][j]) {
                case B_EMPTY:
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);
                    if (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h && is_move_possible(i, j))
                        SDL_RenderCopy(renderer, allowed, NULL, &r);
                    break;
                case B_RED:
                    SDL_RenderCopy(renderer, red, NULL, &r);
                    break;
                case B_BLUE:
                    SDL_RenderCopy(renderer, blue, NULL, &r);
                    break;
                case B_FLIP0:
                    SDL_RenderCopy(renderer, flip0, NULL, &r);
                    break;
                case B_FLIP1:
                    SDL_RenderCopy(renderer, flip1, NULL, &r);
                    break;
                case B_FLIP2:
                    SDL_RenderCopy(renderer, flip2, NULL, &r);
                    break;
            }
            r.x += r.w + 4;
        }
        r.y += r.h + 4;
        r.x = 35;
    }

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
}

// Main
int main() {
    #ifndef EMSCRIPTEN
    std::thread ai_thread([] {
    #endif
        ai_initializing.lock();
        std::cout.setstate(std::ios::failbit);
        std::cerr.setstate(std::ios::failbit);
        init_ai();
        std::cout.clear();
        std::cerr.clear();
        ai_initializing.unlock();
    #ifndef EMSCRIPTEN
    });
    ai_thread.detach();
    #endif

    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    window = SDL_CreateWindow("SDLReversi", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 314, 360,
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

    SDL_RenderSetLogicalSize(renderer, 314, 360);

    assets_load(renderer);
    atexit(assets_free);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = NULL;

    ImGui::StyleColorsClassic();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    ai_level = difficulty;
    animating = false;
    new_game();

    SDL_AddTimer(70, animate, nullptr);

    #ifndef EMSCRIPTEN
    while (!done) game_loop();
    #else
    emscripten_set_main_loop(game_loop, 0, 1);
    #endif
}
