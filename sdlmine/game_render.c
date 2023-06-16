
#include "game_render.h"

#include "assets.h"
#include "logic.h"
#include "main.h"

void draw_bevel(SDL_Renderer * renderer, SDL_Rect rect, int thickness) {
    SDL_SetRenderDrawColor(renderer, 0x5A, 0x5A, 0x5A, 0xFF);
    for (int i = 0; i < thickness; i++) {
        SDL_RenderDrawLine(renderer, rect.x + i, rect.y + i, rect.x + rect.w - i - 1, rect.y + i);
        SDL_RenderDrawLine(renderer, rect.x + i, rect.y + i, rect.x + i, rect.y + rect.h - i - 1);
    }
    SDL_SetRenderDrawColor(renderer, 0xDF, 0xDF, 0xDF, 0xFF);
    for (int i = 0; i < thickness; i++) {
        SDL_RenderDrawLine(renderer, rect.x + rect.w - i - 1, rect.y + i, rect.x + rect.w - i - 1,
                           rect.y + rect.h - i - 1);
        SDL_RenderDrawLine(renderer, rect.x + i, rect.y + rect.h - i - 1, rect.x + rect.w - i - 1,
                           rect.y + rect.h - i - 1);
    }
}

static SDL_Texture * dig(int d) {
    if (black_white) {
        switch (d) {
            case 0:
                return bwdig0011;
            case 1:
                return bwdig0010;
            case 2:
                return bwdig0009;
            case 3:
                return bwdig0008;
            case 4:
                return bwdig0007;
            case 5:
                return bwdig0006;
            case 6:
                return bwdig0005;
            case 7:
                return bwdig0004;
            case 8:
                return bwdig0003;
            case 9:
                return bwdig0002;
        }
    } else {
        switch (d) {
            case 0:
                return cdig0011;
            case 1:
                return cdig0010;
            case 2:
                return cdig0009;
            case 3:
                return cdig0008;
            case 4:
                return cdig0007;
            case 5:
                return cdig0006;
            case 6:
                return cdig0005;
            case 7:
                return cdig0004;
            case 8:
                return cdig0003;
            case 9:
                return cdig0002;
        }
    }
}

void render_3num(SDL_Renderer * renderer, int num, int x, int y) {
    int c1_dig0 = num / 100;
    int c1_dig1 = (num - c1_dig0 * 100) / 10;
    int c1_dig2 = num % 10;
    SDL_Rect rect;
    rect.y = y;
    rect.w = 13;
    rect.h = 23;
    rect.x = x;
    SDL_RenderCopy(renderer, dig(c1_dig0), NULL, &rect);
    rect.x += 13;
    SDL_RenderCopy(renderer, dig(c1_dig1), NULL, &rect);
    rect.x += 13;
    SDL_RenderCopy(renderer, dig(c1_dig2), NULL, &rect);
}

static SDL_Texture * uncovered_tile_texture(int tile_type) {
    if (black_white) {
        switch (tile_type) {
            case 0:
                return bwgam0015;
            case 1:
                return bwgam0014;
            case 2:
                return bwgam0013;
            case 3:
                return bwgam0012;
            case 4:
                return bwgam0011;
            case 5:
                return bwgam0010;
            case 6:
                return bwgam0009;
            case 7:
                return bwgam0008;
            case 8:
                return bwgam0007;
        }
    } else {
        switch (tile_type) {
            case 0:
                return cgam0015;
            case 1:
                return cgam0014;
            case 2:
                return cgam0013;
            case 3:
                return cgam0012;
            case 4:
                return cgam0011;
            case 5:
                return cgam0010;
            case 6:
                return cgam0009;
            case 7:
                return cgam0008;
            case 8:
                return cgam0007;
        }
    }
}

static SDL_Texture * covered_tile_texture(int tile_type, int hovered) {
    if (black_white) {
        switch (tile_type) {
            case VIS_QUESTION:
                return bwgam0002;
            case VIS_COVERED:
                return hovered ? bwgam0015 : bwgam0000;
            case VIS_FLAG:
                return bwgam0001;
            case VIS_WRONG_FLAG:
                return bwgam0004;
            case VIS_LOSING_MINE:
                return bwgam0003;
            case VIS_OTHER_MINE:
                return bwgam0005;
        }
    } else {
        switch (tile_type) {
            case VIS_QUESTION:
                return cgam0002;
            case VIS_COVERED:
                return hovered ? cgam0015 : cgam0000;
            case VIS_FLAG:
                return cgam0001;
            case VIS_WRONG_FLAG:
                return cgam0004;
            case VIS_LOSING_MINE:
                return cgam0003;
            case VIS_OTHER_MINE:
                return cgam0005;
        }
    }
}

void render_minefield(SDL_Renderer * renderer, int offX, int offY, int selCellX, int selCellY) {
    int tile_x = offX;
    int tile_y = offY;
    for (unsigned cur_y = 0; cur_y < minefield_y; cur_y++) {
        for (unsigned cur_x = 0; cur_x < minefield_x; cur_x++) {
            SDL_Rect rect;
            rect.x = tile_x;
            rect.y = tile_y;
            rect.w = 16;
            rect.h = 16;
            int vis = minefield_visible[cur_y * minefield_x + cur_x];
            if (vis == VIS_UNCOVERED) {
                SDL_RenderCopy(renderer, uncovered_tile_texture(minefield[cur_y * minefield_x + cur_x]), NULL, &rect);
            } else {
                int x, y;
                SDL_GetMouseState(&x, &y);
                SDL_RenderCopy(renderer, covered_tile_texture(vis, selCellX == cur_x && selCellY == cur_y), NULL,
                               &rect);
            }
            tile_x += 16;
        }
        tile_x = 13;
        tile_y += 16;
    }
}

void render_face(SDL_Renderer * renderer, char grimace, int offsetX, int offsetY) {
    SDL_Rect rect;
    rect.x = offsetX;
    rect.y = offsetY;
    rect.w = 24;
    rect.h = 24;
    if (black_white) {
        switch (grimace) {
            case 0:
                SDL_RenderCopy(renderer, bwface0004, NULL, &rect);
                break;
            case 1:
                SDL_RenderCopy(renderer, bwface0000, NULL, &rect);
                break;
            case 2:
                SDL_RenderCopy(renderer, bwface0003, NULL, &rect);
                break;
            case 3:
                SDL_RenderCopy(renderer, bwface0002, NULL, &rect);
                break;
            case 4:
                SDL_RenderCopy(renderer, bwface0001, NULL, &rect);
                break;
        }
    } else {
        switch (grimace) {
            case 0:
                SDL_RenderCopy(renderer, cface0004, NULL, &rect);
                break;
            case 1:
                SDL_RenderCopy(renderer, cface0000, NULL, &rect);
                break;
            case 2:
                SDL_RenderCopy(renderer, cface0003, NULL, &rect);
                break;
            case 3:
                SDL_RenderCopy(renderer, cface0002, NULL, &rect);
                break;
            case 4:
                SDL_RenderCopy(renderer, cface0001, NULL, &rect);
                break;
        }
    }
}
