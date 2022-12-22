
#include "leaderboard.h"
#include "game_state.h"
#include "ui.h"

void leaderboardHandleMouseUp(int x, int y) {
    // Check if we clicked outside of the settings window.
    if(x <= LEADERBOARD_WND_OFFSET_X || x >= LEADERBOARD_WND_OFFSET_X + LEADERBOARD_WND_WIDTH ||
       y <= LEADERBOARD_WND_OFFSET_Y || y >= LEADERBOARD_WND_OFFSET_Y + LEADERBOARD_WND_HEIGHT) {
        game.state = STATE_GAME_IDLE;
        return;
    }
}

struct text_row {
    SDL_Texture * no;
    SDL_Texture * points;
    SDL_Texture * moves;
    SDL_Texture * time;
    SDL_Texture * date;
};

static struct text_row leaderboard_data[11];

#define registerEntry(msg, dest) \
    surface = TTF_RenderText_Blended(font, msg, black); \
    leaderboard_data[i].dest = SDL_CreateTextureFromSurface(renderer, surface); \
    SDL_FreeSurface(surface);

void onLeaderboardUpdate() {
    SDL_Color black = {0, 0, 0, 255};
    int i;
    for(i = 0; i < 11; i++) {
        if(leaderboard_data[i].points) SDL_DestroyTexture(leaderboard_data[i].points);
        if(leaderboard_data[i].moves) SDL_DestroyTexture(leaderboard_data[i].moves);
        if(leaderboard_data[i].time) SDL_DestroyTexture(leaderboard_data[i].time);
        if(leaderboard_data[i].date) SDL_DestroyTexture(leaderboard_data[i].date);

        char buf[32];
        SDL_Surface * surface;

        if(i > 0 && game.leaderboard[i - 1].score == 0) {
            // Empty score.
            sprintf(buf, "%d", i);
            registerEntry(buf, no);
            registerEntry("--", points);
            registerEntry("--", moves);
            registerEntry("--", time);
            registerEntry("--", date);
        } else if(i == 0) {
            // Header.
            registerEntry("No.", no);
            registerEntry("Score", points);
            registerEntry("Moves", moves);
            registerEntry("Time", time);
            registerEntry("Date", date);
        } else {
            // Score.
            sprintf(buf, "%d", i);
            registerEntry(buf, no);
            sprintf(buf, "%d", game.leaderboard[i - 1].score);
            registerEntry(buf, points);
            sprintf(buf, "%d", game.leaderboard[i - 1].moves);
            registerEntry(buf, moves);
            int time = game.leaderboard[i - 1].time;
            int minutes = time / 60;
            int seconds = time % 60;
            sprintf(buf, "%d:%02d", minutes, seconds);
            registerEntry(buf, time);
            strftime(buf, 32, "%d/%m/%Y", localtime(&game.leaderboard[i - 1].when));
            registerEntry(buf, date);
        }
    }
}

#undef registerEntry

void leaderboardHandleRender() {
    SDL_Rect rect;
    rect.x = LEADERBOARD_WND_OFFSET_X;
    rect.y = LEADERBOARD_WND_OFFSET_Y;
    rect.w = LEADERBOARD_WND_WIDTH;
    rect.h = LEADERBOARD_WND_HEIGHT;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
    rect.x++; rect.y++;
    rect.w -= 2; rect.h -= 2;
    SDL_SetRenderDrawColor(renderer, 170, 170, 170, 255);
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw a table spanning the entire window.
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    rect.x = LEADERBOARD_WND_OFFSET_X + 10;
    rect.y = LEADERBOARD_WND_OFFSET_Y + 10;
    rect.w = LEADERBOARD_WND_WIDTH - 20;
    rect.h = LEADERBOARD_WND_HEIGHT - 20;
    SDL_RenderDrawRect(renderer, &rect);

    // Draw all table rows.
    for(int i = 0; i < 10; i++) {
        SDL_Rect row;
        SDL_QueryTexture(leaderboard_data[i].no, NULL, NULL, &row.w, &row.h);
        row.x = LEADERBOARD_WND_OFFSET_X + 15;
        row.y = LEADERBOARD_WND_OFFSET_Y + 15 + i * 20;
        SDL_RenderCopy(renderer, leaderboard_data[i].no, NULL, &row);
        row.x += 30;
        SDL_QueryTexture(leaderboard_data[i].points, NULL, NULL, &row.w, &row.h);
        SDL_RenderCopy(renderer, leaderboard_data[i].points, NULL, &row);
        row.x += 45;
        SDL_QueryTexture(leaderboard_data[i].moves, NULL, NULL, &row.w, &row.h);
        SDL_RenderCopy(renderer, leaderboard_data[i].moves, NULL, &row);
        row.x += 50;
        SDL_QueryTexture(leaderboard_data[i].time, NULL, NULL, &row.w, &row.h);
        SDL_RenderCopy(renderer, leaderboard_data[i].time, NULL, &row);
        row.x += 50;
        SDL_QueryTexture(leaderboard_data[i].date, NULL, NULL, &row.w, &row.h);
        SDL_RenderCopy(renderer, leaderboard_data[i].date, NULL, &row);
    }

    // Draw the bars.
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    int x_offset = LEADERBOARD_WND_OFFSET_X;
    int y_offset = LEADERBOARD_WND_OFFSET_Y;
    #define draw_horiz(x) \
        x_offset += x; \
        SDL_RenderDrawLine(renderer, x_offset, LEADERBOARD_WND_OFFSET_Y + 10, x_offset, LEADERBOARD_WND_OFFSET_Y + LEADERBOARD_WND_HEIGHT - 10);
    draw_horiz(40);
    draw_horiz(45);
    draw_horiz(50);
    draw_horiz(50);
    #define draw_vert(x) \
        y_offset += x; \
        SDL_RenderDrawLine(renderer, LEADERBOARD_WND_OFFSET_X + 10, y_offset, LEADERBOARD_WND_OFFSET_X + LEADERBOARD_WND_WIDTH - 10, y_offset);
    draw_vert(33);
    draw_vert(20);
    draw_vert(20);
    draw_vert(20);
    draw_vert(20);
    draw_vert(20);
    draw_vert(20);
    draw_vert(20);
    draw_vert(20);
}
