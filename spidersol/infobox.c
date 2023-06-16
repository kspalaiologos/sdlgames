
#include "infobox.h"

#include "game_state.h"
#include "ui.h"

void handleRenderInfobox() {
    // Draw the status box.
    SDL_Rect rect;
    rect.x = INFOBOX_X_OFFSET;
    rect.y = INFOBOX_Y_OFFSET;
    rect.w = INFOBOX_WIDTH;
    rect.h = INFOBOX_HEIGHT;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
    rect.x++;
    rect.y++;
    rect.w -= 2;
    rect.h -= 2;
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
    SDL_RenderFillRect(renderer, &rect);

    // Render text if required.
    static SDL_Texture * texture = NULL;
    static SDL_Texture * movesTexture = NULL;
    static SDL_Texture * pointsTexture = NULL;
    static SDL_Texture * difficultyTexture = NULL;
    if (game.needsTextRepaint || texture == NULL) {
        if (texture != NULL) {
            // Destroy all textual textures...
            SDL_DestroyTexture(texture);
            SDL_DestroyTexture(movesTexture);
            SDL_DestroyTexture(pointsTexture);
            SDL_DestroyTexture(difficultyTexture);
        }
        char buf[256];
        sprintf(buf, "Time elapsed: %d:%02d", game.time / 60, game.time % 60);
        SDL_Color color = { 255, 255, 255, 255 };
        SDL_Surface * surface = TTF_RenderText_Blended(font, buf, color);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        sprintf(buf, "Moves: %d", game.moves);
        surface = TTF_RenderText_Blended(font, buf, color);
        movesTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        sprintf(buf, "Points: %d", game.points);
        surface = TTF_RenderText_Blended(font, buf, color);
        pointsTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        const char * difficulties[] = { "Easy", "Medium", "Hard" };
        sprintf(buf, "Difficulty: %s", difficulties[game.difficulty - 1]);
        surface = TTF_RenderText_Blended(font, buf, color);
        difficultyTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        game.needsTextRepaint = 0;
    }
    SDL_Rect text_rect;
    text_rect.y = INFOBOX_Y_OFFSET + 10;
    SDL_QueryTexture(texture, NULL, NULL, &text_rect.w, &text_rect.h);
    text_rect.x = INFOBOX_X_OFFSET + (INFOBOX_WIDTH - text_rect.w) / 2;
    SDL_RenderCopy(renderer, texture, NULL, &text_rect);
    text_rect.y += text_rect.h + 10;
    SDL_QueryTexture(movesTexture, NULL, NULL, &text_rect.w, &text_rect.h);
    text_rect.x = INFOBOX_X_OFFSET + (INFOBOX_WIDTH - text_rect.w) / 2;
    SDL_RenderCopy(renderer, movesTexture, NULL, &text_rect);
    text_rect.y += text_rect.h + 10;
    SDL_QueryTexture(pointsTexture, NULL, NULL, &text_rect.w, &text_rect.h);
    text_rect.x = INFOBOX_X_OFFSET + (INFOBOX_WIDTH - text_rect.w) / 2;
    SDL_RenderCopy(renderer, pointsTexture, NULL, &text_rect);
    text_rect.y += text_rect.h + 10;
    SDL_QueryTexture(difficultyTexture, NULL, NULL, &text_rect.w, &text_rect.h);
    text_rect.x = INFOBOX_X_OFFSET + (INFOBOX_WIDTH - text_rect.w) / 2;
    SDL_RenderCopy(renderer, difficultyTexture, NULL, &text_rect);
}
