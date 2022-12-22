
#include "deal.h"

#include "game_state.h"
#include "undo.h"
#include "win.h"

static int dealX, dealY, row_dealt;

static void shuffle_card(int * dsuit, int * dvalue) {
    int suit, value;

    if (game.difficulty == 3) {
        // Four suits.
        for (;;) {
            suit = rand() % 4;
            value = rand() % 13;
            if (game.counts[suit][value] < 2) {
                game.counts[suit][value]++;
                break;
            }
        }
    } else if (game.difficulty == 2) {
        // Two suits: need to be careful. Suit must be either 1 or 2, we're simulating 4 decks.
        for (;;) {
            suit = (rand() % 2) + 1;
            value = rand() % 13;
            if (game.counts[suit][value] < 4) {
                game.counts[suit][value]++;
                break;
            }
        }
    } else if (game.difficulty == 1) {
        // One suit: Suit must be 3, we're simulating 8 decks.
        for (;;) {
            suit = 3;
            value = rand() % 13;
            if (game.counts[suit][value] < 8) {
                game.counts[suit][value]++;
                break;
            }
        }
    }

    *dsuit = suit;
    *dvalue = value;
}

void deal_row(void) {
    if (row_dealt == 10) {
        row_dealt = 0;
        game.state = STATE_GAME_IDLE;
        sequencePoint();
        try_collapse_row();
        boardStateChangedDispatcher();
        return;
    }

    int suit = game.extracards[game.remainingExtraDeals][row_dealt].suit;
    int value = game.extracards[game.remainingExtraDeals][row_dealt].value;

    game.stacks[row_dealt].cards[game.stacks[row_dealt].num_cards].suit = suit;
    game.stacks[row_dealt].cards[game.stacks[row_dealt].num_cards].value = value;
    game.stacks[row_dealt].num_cards++;
    game.stacks[row_dealt].visible_offset++;

    row_dealt++;
}

void deal(void) {
    // Perform a single step of dealing the deck.
    // This function is called every frame while the game is in the deal state.
    // It should deal six cards to first four stacks and five cards to the last six stacks.
    // When the deal is complete, it should switch the game state to STATE_GAME_IDLE.

    int suit, value;
    shuffle_card(&suit, &value);

    game.stacks[dealX].cards[game.stacks[dealX].num_cards].suit = suit;
    game.stacks[dealX].cards[game.stacks[dealX].num_cards].value = value;
    game.stacks[dealX].num_cards++;

    // Check if we dealt the last card in this column:
    if ((dealX <= 3 && dealY == 5) || (dealX > 3 && dealY == 4)) {
        game.stacks[dealX].visible_offset = 1;
    }

    dealX++;
    if (dealX == 10) {
        dealX = 0;
        dealY++;
    }

    if (dealY == 5 && dealX == 4) {
        game.state = STATE_GAME_IDLE;
        // Deal the extra deals.
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                shuffle_card(&suit, &value);
                game.extracards[i][j].suit = suit;
                game.extracards[i][j].value = value;
            }
        }
        dealX = 0;
        dealY = 0;
        sequencePoint();
        boardStateChangedDispatcher();
    }
}
