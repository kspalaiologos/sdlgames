
#include "undo.h"

#include "game_state.h"
#include "win.h"

void sequencePoint() {
    if (game.timeDelta == vector_size(game.stackHistory) - 1) {
        game.timeDelta++;
        struct historical_state s;
        for (int i = 0; i < 10; i++) s.stacks[i] = game.stacks[i];
        s.points = game.points;
        s.remainingExtraDeals = game.remainingExtraDeals;
        vector_push_back(game.stackHistory, s);
    } else {
        // Overwrite history: pop timeDelta entries.
        while (game.timeDelta != vector_size(game.stackHistory) - 1) vector_pop_back(game.stackHistory);
        sequencePoint();
    }
}

void undo() {
    if (game.timeDelta != 0) {
        game.timeDelta--;
        struct historical_state s = game.stackHistory[game.timeDelta];
        game.points = s.points;
        game.remainingExtraDeals = s.remainingExtraDeals;
        for (int i = 0; i < 10; i++) {
            game.stacks[i] = s.stacks[i];
        }
        boardStateChangedDispatcher();
    }
}

void redo() {
    if (vector_size(game.stackHistory) > game.timeDelta + 1) {
        struct historical_state s = game.stackHistory[++game.timeDelta];
        game.points = s.points;
        game.remainingExtraDeals = s.remainingExtraDeals;
        for (int i = 0; i < 10; i++) {
            game.stacks[i] = s.stacks[i];
        }
        try_collapse_row();
        boardStateChangedDispatcher();
    }
}
