
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

    for (int i = 0; i < 10; i++) {
        if (game.stacks[i].num_cards == 0) return 1;
    }

    // Compute the offsets in each stack that give a consecutive column slice.
    int offsets[10];
    compute_offsets(offsets);

    // Check if there are any moves possible that won't just end up in a loop.
    // E.g., if we move a 2 to a 3, but the 2 was previously on a 3, this makes no sense unless an entire stack is
    // getting folded.

    // In other words: Move only full stacks as defined from offsets[i] to stacks[i].num_cards, UNLESS the last card is
    // an Ace and this operation will result in a full stack from king, queen, jack, etc... until an ace.

    // Start with the easy case first: Check if any _full_ stack can be moved anywhere.

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (i == j || offsets[i] == -1) continue;

            // Can be moved!
            if (game.stacks[i].cards[offsets[i]].value + 1 ==
                    game.stacks[j].cards[game.stacks[j].num_cards - 1].value &&
                game.stacks[i].cards[offsets[i]].suit == game.stacks[j].cards[game.stacks[j].num_cards - 1].suit)
                return 1;
        }
    }

    // The difficult case:
    // Mask out the stacks that start with a king and the stacks that end with an ace.

    int ends[10], begins[10];
    int ec[10], bc[10];

    for (int i = 0; i < 10; i++) {
        ends[i] = -1;
        begins[i] = -1;
        ec[i] = -1;
        bc[i] = -1;
    }

    for (int i = 0; i < 10; i++) {
        if (game.stacks[i].num_cards == 0 || offsets[i] == -1) continue;

        if (game.stacks[i].cards[offsets[i]].value == 0) {
            ends[i] = game.stacks[i].cards[game.stacks[i].num_cards - 1].value;
            ec[i] = game.stacks[i].cards[game.stacks[i].num_cards - 1].suit;
        }

        if (game.stacks[i].cards[game.stacks[i].num_cards - 1].value == 12) {
            begins[i] = game.stacks[i].cards[offsets[i]].value;
            bc[i] = game.stacks[i].cards[offsets[i]].suit;
        }
    }

    // Determine if there is such ends[i] and begins[j] that ends[i] <= begins[j]
    // and i != j. If so, return 1.

    for (int i = 0; i < 10; i++) {
        if (ends[i] == -1) continue;

        for (int j = 0; j < 10; j++) {
            // stack begins with begins[j] and ends with Ace.
            // stack starts with King and ends with ends[i].
            // meaning that we're seeking for overlap in these two stacks.

            // assume we have a run from K to 6 and from 8 to A.
            // begins = 8, ends = 6, but it's reverse so ends > begins.

            if (begins[j] == -1) continue;

            if (i == j) continue;

            if (ec[i] == bc[j] && begins[j] <= ends[i]) {
                return 1;
            }
        }
    }

    // Assume no legal moves.

    return 0;
}
