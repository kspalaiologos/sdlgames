
#include "win.h"

#include "game_state.h"
#include "leaderboard.h"
#include "undo.h"

void try_collapse_row() {
    // Check if one row has DISCOVERED cards from K through Q, J, to A
    // of the same color at the end. If so, remove them all and add a king of the
    // respective color to wonKings.

    for (int i = 0; i < 10; i++) {
        int numCards = game.stacks[i].num_cards;
        if (numCards < 13 || game.stacks[i].visible_offset < 13) {
            continue;
        }

        int suit = game.stacks[i].cards[numCards - 1].suit;
        int value = game.stacks[i].cards[numCards - 1].value;
        if (value != 12) {
            continue;
        }

        int j;
        for (j = 0; j < 13; j++) {
            if (game.stacks[i].cards[numCards - 1 - j].suit != suit) {
                break;
            }
            if (game.stacks[i].cards[numCards - 1 - j].value != 12 - j) {
                break;
            }
        }

        if (j == 13) {
            // We have a full row!
            game.stacks[i].num_cards -= 13;
            game.stacks[i].visible_offset -= 13;

            if (game.stacks[i].visible_offset <= 0) {
                game.stacks[i].visible_offset = 1;
            }

            for (j = 0; j < 8; j++) {
                if (game.wonKings[j] == -1) {
                    game.wonKings[j] = suit;
                    break;
                }
            }

            game.points += 100;

            boardStateChangedDispatcher();
        }
    }
}

int handle_victory() {
    boardStateChangedDispatcher();

    // Add to the highest scores leaderboard?
    // Check if there are empty leaderboard entries.
    int i;
    for (i = 0; i < 10; i++) {
        if (game.leaderboard[i].when == 0) {
            break;
        }
    }

    // If so, fill the slot with our score.
    if (i < 10) {
        game.leaderboard[i].score = game.points;
        game.leaderboard[i].moves = game.moves;
        game.leaderboard[i].time = game.time;
        game.leaderboard[i].when = time(NULL);
        onLeaderboardUpdate();
    } else {
        // Otherwise, check if our score is higher than the lowest score.
        int lowest = 0;
        for (i = 0; i < 10; i++) {
            if (game.leaderboard[i].score < game.points) {
                lowest = i;
            }
        }

        if (game.points > game.leaderboard[lowest].score) {
            game.leaderboard[lowest].score = game.points;
            game.leaderboard[lowest].moves = game.moves;
            game.leaderboard[lowest].time = game.time;
            game.leaderboard[lowest].when = time(NULL);
            onLeaderboardUpdate();
        }
    }
}

#include "ui.h"

static SDL_Rect gameOverBox;

// Draw a box containing GAME OVER text.
void handle_loss() {
    boardStateChangedDispatcher();

    SDL_Texture * game_over;
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Surface * game_over_surface = TTF_RenderText_Blended(big_font, "GAME OVER", white);
    game_over = SDL_CreateTextureFromSurface(renderer, game_over_surface);
    SDL_FreeSurface(game_over_surface);
    SDL_Rect box;
    SDL_QueryTexture(game_over, NULL, NULL, &box.w, &box.h);
    box.w += 20;
    box.h += 20;
    box.x = (WIN_WIDTH - box.w) / 2;
    box.y = (WIN_HEIGHT - box.h) / 2;
    gameOverBox = box;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
    box.x++;
    box.y++;
    box.w -= 2;
    box.h -= 2;
    SDL_RenderFillRect(renderer, &box);
    box.w += 2;
    box.h += 2;
    box.x += 9;
    box.y += 9;
    SDL_QueryTexture(game_over, NULL, NULL, &box.w, &box.h);
    SDL_RenderCopy(renderer, game_over, NULL, &box);
}

void handleLossMouseUp(int x, int y) {
    // check if x and y are outside gameOverBox.
    if (x <= gameOverBox.x || x >= gameOverBox.x + gameOverBox.w || y <= gameOverBox.y ||
        y >= gameOverBox.y + gameOverBox.h) {
        game.state = STATE_GAME_IDLE;
        undo();
        return;
    }
}
