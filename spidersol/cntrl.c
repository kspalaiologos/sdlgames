
#include "cntrl.h"

#include "engine.h"
#include "game_state.h"
#include "undo.h"
#include "win.h"

int selectedColumn = -1, selectedIndex = 1, selectedSrcColumn = -1, selectedSrcColumnIndex = 1;

void cntrlHandleBoardStateChange() {
    // Reset everything.
    selectedColumn = -1;
    selectedSrcColumn = -1;
    selectedIndex = 1;
    selectedSrcColumnIndex = 1;
}

void cntrlRender() {
    // Render a yellow box around selectedColumn up to selectedIndex.
    if (selectedColumn != -1) {
        if (game.stacks[selectedColumn].num_cards != 0) {
            SDL_Rect r;
            r.x = game.stacks[selectedColumn].cards[game.stacks[selectedColumn].num_cards - selectedIndex].x - 5;
            r.y = game.stacks[selectedColumn].cards[game.stacks[selectedColumn].num_cards - selectedIndex].y - 5;
            r.w = CARD_WIDTH + 10;
            r.h = VISIBLE_Y_OFFSET * (selectedIndex - 1) + CARD_HEIGHT + 10;
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &r);
        } else {
            SDL_Rect r;
            r.x = STACK_X_OFFSET + selectedColumn * STACK_X_SPACING - 5;
            r.y = STACK_Y_OFFSET - 5;
            r.w = CARD_WIDTH + 10;
            r.h = CARD_HEIGHT + 10;
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &r);
        }
    }

    if (selectedSrcColumn != -1) {
        if (game.stacks[selectedSrcColumn].num_cards != 0) {
            SDL_Rect r;
            r.x = game.stacks[selectedSrcColumn]
                      .cards[game.stacks[selectedSrcColumn].num_cards - selectedSrcColumnIndex]
                      .x -
                  5;
            r.y = game.stacks[selectedSrcColumn]
                      .cards[game.stacks[selectedSrcColumn].num_cards - selectedSrcColumnIndex]
                      .y -
                  5;
            r.w = CARD_WIDTH + 10;
            r.h = VISIBLE_Y_OFFSET * (selectedSrcColumnIndex - 1) + CARD_HEIGHT + 10;
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &r);
        } else {
            SDL_Rect r;
            r.x = STACK_X_OFFSET + selectedSrcColumn * STACK_X_SPACING - 5;
            r.y = STACK_Y_OFFSET - 5;
            r.w = CARD_WIDTH + 10;
            r.h = CARD_HEIGHT + 10;
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &r);
        }
    }
}

int get_this_offset(int off) {
    int offsets[10];
    compute_offsets(offsets);
    for (int i = 0; i < 10; i++) {
        if (offsets[i] != -1 && offsets[i] < game.stacks[i].num_cards - game.stacks[i].visible_offset)
            offsets[i] = game.stacks[i].num_cards - game.stacks[i].visible_offset;
    }
    return offsets[off];
}

int can_pick_any_row() {
    for (int i = 0; i < 10; i++)
        if (game.stacks[i].num_cards > 0) return 1;
    return 0;
}

void try_move() {
    if (selectedColumn != selectedSrcColumn &&
        ((game.stacks[selectedColumn].num_cards == 0) ||
         (game.stacks[selectedColumn].cards[game.stacks[selectedColumn].num_cards - 1].suit ==
              game.stacks[selectedSrcColumn]
                  .cards[game.stacks[selectedSrcColumn].num_cards - selectedSrcColumnIndex]
                  .suit &&
          game.stacks[selectedColumn].cards[game.stacks[selectedColumn].num_cards - 1].value ==
              game.stacks[selectedSrcColumn]
                      .cards[game.stacks[selectedSrcColumn].num_cards - selectedSrcColumnIndex]
                      .value -
                  1))) {
        // OK, we can move.
        selectedSrcColumnIndex = game.stacks[selectedSrcColumn].num_cards - selectedSrcColumnIndex;

        int moved_cards = game.stacks[selectedSrcColumn].num_cards - selectedSrcColumnIndex;
        for (int i = selectedSrcColumnIndex; i < game.stacks[selectedSrcColumn].num_cards; i++) {
            game.stacks[selectedColumn].cards[game.stacks[selectedColumn].num_cards] =
                game.stacks[selectedSrcColumn].cards[i];
            game.stacks[selectedColumn].num_cards++;
            game.stacks[selectedColumn].visible_offset++;
        }

        game.stacks[selectedSrcColumn].visible_offset -= moved_cards;
        if (game.stacks[selectedSrcColumn].visible_offset <= 0) game.stacks[selectedSrcColumn].visible_offset = 1;
        game.stacks[selectedSrcColumn].num_cards = selectedSrcColumnIndex;

        game.moves++;
        game.points -= 4 - game.difficulty;
        game.needsTextRepaint = 1;

        try_collapse_row();
        sequencePoint();
        boardStateChangedDispatcher();
    } else {
        cntrlHandleBoardStateChange();
    }
}

void cntrlHandleKey(SDL_Keysym sym) {
    if (game.state == STATE_GAME_IDLE) {
        switch (sym.sym) {
            case SDLK_u:
                undo();
                break;
            case SDLK_r:
                redo();
                break;
            case SDLK_TAB:
                if (can_pick_any_row()) {
                    selectedIndex = 1;
                    if (selectedColumn == -1) {
                        selectedColumn = 0;
                    } else {
                        // Needs an empty column to hang. Can't happen in idle mode.
                        if (selectedSrcColumn == -1) {
                            do {
                                selectedColumn++;
                                if (selectedColumn == 10) selectedColumn = 0;
                            } while (get_this_offset(selectedColumn) == -1);
                        } else {
                            selectedColumn++;
                            if (selectedColumn == 10) selectedColumn = 0;
                            if (selectedColumn == selectedSrcColumn) {
                                selectedColumn++;
                                if (selectedColumn == 10) selectedColumn = 0;
                            }
                        }
                    }
                }
                break;
            case SDLK_SPACE:
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
                break;
            case SDLK_RETURN:
                if (selectedColumn != -1) {
                    if (selectedSrcColumn == -1) {
                        selectedSrcColumn = selectedColumn;
                        selectedColumn = -1;
                        selectedSrcColumnIndex = selectedIndex;
                        selectedIndex = 1;
                    } else {
                        if (selectedColumn != selectedSrcColumn) {
                            // Make a move.
                            try_move();
                        }
                        selectedSrcColumn = -1;
                    }
                }
                break;
            case SDLK_DOWN:
                if (selectedSrcColumn == -1 && selectedColumn != -1) {
                    if (selectedIndex > 1) selectedIndex--;
                }
                break;
            case SDLK_UP:
                if (selectedSrcColumn == -1 && selectedColumn != -1) {
                    int off = game.stacks[selectedColumn].num_cards - get_this_offset(selectedColumn);
                    selectedIndex++;
                    if (selectedIndex >= off) selectedIndex = off;
                }
                break;
            default:
                // Ctrl + N - new game.
                if (sym.sym == SDLK_n && sym.mod & KMOD_CTRL) {
                    new_game();
                }

                // Ctrl + R - restart game.
                if (sym.sym == SDLK_r && sym.mod & KMOD_CTRL) {
                    reset_game();
                }
        }
    }
}
