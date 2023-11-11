
#include "engine.h"

#include "game_state.h"

void compute_offsets(int * offsets) {
    for (int i = 0; i < 10; i++) offsets[i] = 0;
    for (int i = 0; i < 10; i++) {
        if (game.stacks[i].num_cards == 0) {
            offsets[i] = -1;
            continue;
        }
        if (game.stacks[i].num_cards == 1) {
            offsets[i] = 0;
            continue;
        }

        while (game.stacks[i].num_cards - 2 - offsets[i] >= 0 &&
               game.stacks[i].cards[game.stacks[i].num_cards - 1 - offsets[i]].value - 1 ==
                   game.stacks[i].cards[game.stacks[i].num_cards - 2 - offsets[i]].value &&
               game.stacks[i].cards[game.stacks[i].num_cards - 1 - offsets[i]].suit ==
                   game.stacks[i].cards[game.stacks[i].num_cards - 2 - offsets[i]].suit) {
            offsets[i]++;
        }

        offsets[i] = game.stacks[i].num_cards - offsets[i] - 1;
    }
}

int has_moves() {
    // Return true if there are any moves left.
    // Don't count circular moves.
    // Assumes we are called AFTER fold checking etc.. is done.

    if (game.remainingExtraDeals > 0) return 1;

    // TODO.
    return 1;
}
