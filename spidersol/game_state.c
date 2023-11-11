
#include "game_state.h"

#include "undo.h"

#include <stdio.h>

static char * fs_name() { return "solitaire.dat"; }

static void fs_flush_data(FILE * f) { fflush(f); }

struct game_data game = { .remainingExtraDeals = 5,
                          .timeDelta = -1,
                          .selected_card_back = 9,
                          .wonKings = { -1, -1, -1, -1, -1, -1, -1, -1 },
                          .state = STATE_GAME_DEAL };

void reset_game() {
    srand(game.lastseed);
    for (int i = 0; i < 8; i++) game.wonKings[i] = -1;
    for (int i = 0; i < 10; i++) {
        game.stacks[i].num_cards = 0;
        game.stacks[i].visible_offset = 0;
    }
    game.points = 500 * game.difficulty;
    game.time = 0;
    game.moves = 0;
    game.needsTextRepaint = 1;
    game.state = STATE_GAME_DEAL;
    game.timeDelta = -1;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 13; j++) game.counts[i][j] = 0;
    game.remainingExtraDeals = 5;
    vector_free(game.stackHistory);
    game.stackHistory = NULL;
    boardStateChangedDispatcher();
}

void new_game() {
    game.lastseed = time(NULL);
    reset_game();
}

void load_storage() {
    FILE * f = fopen(fs_name(), "rb");
    if (f == NULL) {
        game.difficulty = 3;
        game.selected_card_back = 9;
        new_game();
        return;
    }

    fscanf(f, "%d %d \n", &game.selected_card_back, &game.difficulty);
    for (int i = 0; i < 8; i++) {
        fscanf(f, "%d ", &game.wonKings[i]);
    }
    fscanf(f, "\n");

    int hasState;
    fscanf(f, "%d\n", &hasState);

    if (hasState) {
        fscanf(f, "%d %d %d %d %d \n", &game.points, &game.time, &game.moves, &game.prevdiff,
               &game.remainingExtraDeals);
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                fscanf(f, "%d %d ", &(game.extracards[i][j].suit), &(game.extracards[i][j].value));
            }
            fscanf(f, "\n");
        }
        for (int i = 0; i < 10; i++) {
            fscanf(f, "%d %d ", &game.stacks[i].num_cards, &game.stacks[i].visible_offset);
            for (int j = 0; j < game.stacks[i].num_cards; j++) {
                fscanf(f, "%d %d ", &game.stacks[i].cards[j].suit, &game.stacks[i].cards[j].value);
            }
            fscanf(f, "\n");
        }

        game.state = STATE_GAME_IDLE;
        sequencePoint();
        boardStateChangedDispatcher();
    } else {
        new_game();
    }
}

void save_storage() {
    FILE * f = fopen(fs_name(), "wb");
    if (f == NULL) {
        return;
    }

    fprintf(f, "%d %d \n", game.selected_card_back, game.difficulty);
    for (int i = 0; i < 8; i++) {
        fprintf(f, "%d ", game.wonKings[i]);
    }
    fprintf(f, "\n");

    // Synchronise the game state only if we're in the IDLE state.
    if (game.state == STATE_GAME_IDLE) {
        fprintf(f, "1\n");
        fprintf(f, "%d %d %d %d %d \n", game.points, game.time, game.moves, game.prevdiff, game.remainingExtraDeals);
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                fprintf(f, "%d %d ", game.extracards[i][j].suit, game.extracards[i][j].value);
            }
            fprintf(f, "\n");
        }
        for (int i = 0; i < 10; i++) {
            fprintf(f, "%d %d ", game.stacks[i].num_cards, game.stacks[i].visible_offset);
            for (int j = 0; j < game.stacks[i].num_cards; j++) {
                fprintf(f, "%d %d ", game.stacks[i].cards[j].suit, game.stacks[i].cards[j].value);
            }
            fprintf(f, "\n");
        }
    } else {
        fprintf(f, "0\n");
    }

    fs_flush_data(f);
    fclose(f);
}

#include "cntrl.h"

void boardStateChangedDispatcher() { cntrlHandleBoardStateChange(); }
