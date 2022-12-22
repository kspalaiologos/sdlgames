
#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#include "vector.h"
#include <time.h>

enum {
    STATE_GAME_IDLE,
    STATE_GAME_DEAL,
    STATE_GAME_DRAGGING_STACK,
    STATE_GAME_DEALING_ROW,
    STATE_GAME_SETTINGS,
    STATE_GAME_FIREWORKS,
    STATE_GAME_LEADERBOARD,
    STATE_GAME_LOST
};

struct card {
    // 0 = clubs, 1 = diamonds, 2 = hearts, 3 = spades
    int suit;
    // 0 = king, 1 = queen, 2 = jack, 3 = 10, 4 = 9, 5 = 8, 6 = 7, 7 = 6, 8 = 5, 9 = 4, 10 = 3, 11 = 2, 12 = ace
    // ordered this way to make it easier to check if a card can be placed on another card
    int value;

    // Position of this card on the screen.
    int x, y;
};

struct stack {
    struct card cards[32];
    int num_cards;
    int visible_offset;
};

struct historical_state {
    struct stack stacks[10];
    int points;
    int remainingExtraDeals;
};

struct leaderboard_entry {
    int score;
    int moves;
    int time;
    time_t when;
};

struct game_data {
    int remainingExtraDeals;
    struct card extracards[5][10];

    vector(struct historical_state) stackHistory;
    int timeDelta;

    int state;

    int selected_card_back;

    int wonKings[8];

    time_t lastseed;

    struct stack stacks[10];

    int points, time, moves, difficulty, prevdiff;
    int needsTextRepaint;

    int counts[4][13];

    struct leaderboard_entry leaderboard[10];
};

extern struct game_data game;

void reset_game();
void new_game();

void load_storage();
void save_storage();

void boardStateChangedDispatcher();

#endif
