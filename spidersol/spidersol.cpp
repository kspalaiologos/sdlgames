
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern "C" {
#include "cntrl.h"
#include "deal.h"
#include "engine.h"
#include "game_state.h"
#include "infobox.h"
#include "ui.h"
#include "undo.h"
#include "vector.h"
#include "win.h"
}

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl2.h"
#include "../imgui/imgui_impl_sdlrenderer2.h"

#define EXTRA_CARDS_X_OFFSET 740
#define EXTRA_CARDS_Y_OFFSET 570
#define EXTRA_CARDS_X_SPACING 10

#define KINGS_X_OFFSET 50
#define KINGS_Y_OFFSET 570
#define KINGS_X_SPACING 10

#define MENU_HEIGHT 25
#define MENU_PADDING 5

ImGuiIO * io;

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

void onclickQuit() { exit(0); }

#ifdef EMSCRIPTEN
    #include <emscripten.h>
    #include <emscripten/html5.h>
#endif

SDL_bool done = SDL_FALSE;
uint64_t frame_start;
int secondsCurrent;

void main_loop(void) {
    #ifdef EMSCRIPTEN
    if(done) emscripten_cancel_main_loop();
    #endif
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
                if (io->WantCaptureMouse) break;
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
                if (io->WantCaptureMouse) break;
                if (game.state == STATE_GAME_DRAGGING_STACK) {
                    dragging_stack_x = event.motion.x;
                    dragging_stack_y = event.motion.y;
                }
                break;
            case SDL_KEYDOWN:
                if (io->WantCaptureKeyboard) break;
                cntrlHandleKey(event.key.keysym);
                break;
            case SDL_MOUSEBUTTONUP:
                if (io->WantCaptureMouse) break;
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
                            }
                            break;
                        case STATE_GAME_DRAGGING_STACK:
                            drop_stack(event.button.x, event.button.y);
                            game.state = STATE_GAME_IDLE;
                            break;
                        case STATE_GAME_LOST:
                            handleLossMouseUp(event.button.x, event.button.y);
                            break;
                    }
                }
                break;
        }
        if (ImGui_ImplSDL2_ProcessEvent(&event)) continue;
        if (event.type == SDL_QUIT ||
            (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window)))
            done = SDL_TRUE;
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
            case STATE_GAME_LOST:
                handle_loss();
                break;
        }

        tick_fireworks(frame_end - frame_start);

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Game")) {
                if (ImGui::MenuItem("New Game", "Ctrl+N")) {
                    new_game();
                }
                if (ImGui::MenuItem("Restart", "Ctrl+X")) {
                    reset_game();
                }
                if (ImGui::MenuItem("Undo", "Ctrl+U")) {
                    undo();
                }
                if (ImGui::MenuItem("Redo", "Ctrl+R")) {
                    redo();
                }
                if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
                    done = SDL_TRUE;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Difficulty")) {
                if (ImGui::MenuItem("Easy: One suit", NULL, game.difficulty == 1)) {
                    game.difficulty = 1;
                    new_game();
                }
                if (ImGui::MenuItem("Medium: Two suits", NULL, game.difficulty == 2)) {
                    game.difficulty = 2;
                    new_game();
                }
                if (ImGui::MenuItem("Hard: Four suits", NULL, game.difficulty == 3)) {
                    game.difficulty = 3;
                    new_game();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Card Back")) {
                #define GEN_BACK(name, id) if (ImGui::MenuItem(name, NULL, game.selected_card_back == id)) game.selected_card_back = id;
                GEN_BACK("Floral", 0);
                GEN_BACK("Fish (blue)", 1);
                GEN_BACK("Bricks", 2);
                GEN_BACK("Acorns", 3);
                GEN_BACK("Robot", 4);
                GEN_BACK("Fish (cyan)", 5);
                GEN_BACK("Dots", 6);
                GEN_BACK("Castle", 7);
                GEN_BACK("Fabric", 8);
                GEN_BACK("Roses", 9);
                GEN_BACK("Beach", 10);
                GEN_BACK("Shell", 11);
                GEN_BACK("Fish (yellow)", 12);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
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

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    if (create_window()) return 1;

    load_textures();

    load_storage();
    atexit(save_storage);

    frame_start = SDL_GetTicks64();
    secondsCurrent = time(NULL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->IniFilename = NULL;
    io->LogFilename = NULL;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    #ifndef EMSCRIPTEN
    while (!done) main_loop();
    #else
    emscripten_set_main_loop(main_loop, 0, 1);
    #endif

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
