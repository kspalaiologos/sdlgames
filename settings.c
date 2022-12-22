
#include "settings.h"
#include "game_state.h"
#include "ui.h"

void settingsHandleMouseUp(int x, int y) {
    // Check if we clicked outside of the settings window.
    if(x <= SETTINGS_WND_OFFSET_X || x >= SETTINGS_WND_OFFSET_X + SETTINGS_WND_WIDTH ||
       y <= SETTINGS_WND_OFFSET_Y || y >= SETTINGS_WND_OFFSET_Y + SETTINGS_WND_HEIGHT) {
        // OK, assume that we save the settings and proceed.
        if(game.difficulty != game.prevdiff) {
            // Reset the game.
            new_game();
        } else {
            game.state = STATE_GAME_IDLE;
        }
        return;
    }

    // Check if we clicked on the card back selector.
    if(x > SETTINGS_WND_OFFSET_X + 10 && x < SETTINGS_WND_OFFSET_X + 10 + CARD_WIDTH &&
       y > SETTINGS_WND_OFFSET_Y + 10 && y < SETTINGS_WND_OFFSET_Y + 10 + CARD_HEIGHT) {
        game.selected_card_back = (game.selected_card_back + 1) % 13;
    }

    // Check if we clicked on the difficulty selector.
    int diffwidth = 0;
    switch(game.difficulty) {
        case 1: diffwidth = CARD_WIDTH + SETTINGS_CARDS_X_SPACING; break;
        case 2: diffwidth = CARD_WIDTH * 2 + SETTINGS_CARDS_X_SPACING * 2; break;
        case 3: diffwidth = CARD_WIDTH * 4 + SETTINGS_CARDS_X_SPACING * 4; break;
    }
    int baseX = SETTINGS_WND_OFFSET_X + CARD_WIDTH + 80, baseY = SETTINGS_WND_OFFSET_Y + 10;
    if(x > baseX && x < baseX + diffwidth &&
       y > baseY && y < baseY + CARD_HEIGHT) {
        game.difficulty = (game.difficulty % 3) + 1;
    }
}

void settingsHandleRender() {
    SDL_Rect rect;
    rect.x = SETTINGS_WND_OFFSET_X;
    rect.y = SETTINGS_WND_OFFSET_Y;
    rect.w = SETTINGS_WND_WIDTH;
    rect.h = SETTINGS_WND_HEIGHT;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
    rect.x++; rect.y++;
    rect.w -= 2; rect.h -= 2;
    SDL_SetRenderDrawColor(renderer, 170, 170, 170, 255);
    SDL_RenderFillRect(renderer, &rect);
    static SDL_Texture *selBackText = NULL;
    static SDL_Texture *diffText = NULL;
    if(selBackText == NULL) {
        SDL_Color color = {0, 0, 0, 255};
        SDL_Surface *surface = TTF_RenderText_Blended(font, "Selected card back:", color);
        selBackText = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        surface = TTF_RenderText_Blended(font, "Difficulty:", color);
        diffText = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    SDL_Rect text_rect;
    text_rect.y = SETTINGS_WND_OFFSET_Y + 10;
    text_rect.x = SETTINGS_WND_OFFSET_X + 10;
    SDL_QueryTexture(selBackText, NULL, NULL, &text_rect.w, &text_rect.h);
    SDL_RenderCopy(renderer, selBackText, NULL, &text_rect);
    text_rect.y = SETTINGS_WND_OFFSET_Y + 30;
    text_rect.w = CARD_WIDTH;
    text_rect.h = CARD_HEIGHT;
    SDL_RenderCopy(renderer, card_backs[game.selected_card_back], NULL, &text_rect);
    text_rect.x = SETTINGS_WND_OFFSET_X + CARD_WIDTH + 80;
    text_rect.y = SETTINGS_WND_OFFSET_Y + 10;
    SDL_QueryTexture(diffText, NULL, NULL, &text_rect.w, &text_rect.h);
    SDL_RenderCopy(renderer, diffText, NULL, &text_rect);
    text_rect.y += 20;
    text_rect.w = CARD_WIDTH;
    text_rect.h = CARD_HEIGHT;
    render_card(text_rect.x, text_rect.y, 3, 0);
    text_rect.x += SETTINGS_CARDS_X_SPACING;
    switch(game.difficulty) {
        case 2:
            render_card(text_rect.x, text_rect.y, 0, 0);
            text_rect.x += SETTINGS_CARDS_X_SPACING;
            break;
        case 3:
            render_card(text_rect.x, text_rect.y, 0, 0);
            text_rect.x += SETTINGS_CARDS_X_SPACING;
            render_card(text_rect.x, text_rect.y, 1, 0);
            text_rect.x += SETTINGS_CARDS_X_SPACING;
            render_card(text_rect.x, text_rect.y, 2, 0);
            break;
    }
}
