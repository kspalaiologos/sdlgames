
#include "ui.h"
#include "asset/assets.h"

SDL_Window * window;
SDL_Renderer * renderer;

static SDL_Texture * firework_brush;

SDL_Texture * card_backs[13];
SDL_Texture * card_faces[4][13];

SDL_Texture * background, * spiderCard;

TTF_Font * font;
TTF_Font * big_font;

#include "vector.h"
#include "game_state.h"

struct firework {
    SDL_Color color;
    float xvel, yvel;
    float x, y;
    int isrocket, lifetime;
};

static int fwbkg = 0;

static vector(struct firework) fireworks;

// Gravity.
#define ACCEL 0.0001f

void render_fireworks() {
    static unsigned counter = 0;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, fwbkg);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, NULL);

    if(fwbkg <= 220 && game.state == STATE_GAME_FIREWORKS) {
        fwbkg += 4;
    } else if(fwbkg >= 4 && game.state != STATE_GAME_FIREWORKS) {
        fwbkg -= 4;
    }
        
    if((counter++) % (FPS * 2) != 0) {
        return;
    }

    // Generate a random bright color.
    SDL_Color color;
    color.r = rand() % 127 + 128;
    color.g = rand() % 127 + 128;
    color.b = rand() % 127 + 128;
    color.a = 255;

    int x = rand() % (WIN_WIDTH / 2) + (WIN_WIDTH / 4);
    int y = WIN_HEIGHT - 1;

    struct firework fw = { color, 0, 0, x, y };

    // Make the best attempt to shoot it towards the middle.
    float theta = atan2(WIN_HEIGHT / 2 - y, WIN_WIDTH / 2 - x);
    fw.xvel = cos(theta) * 0.3f;
    fw.yvel = sin(theta) * 0.3f;

    fw.isrocket = 1;
    fw.lifetime = 1200;

    vector_push_back(fireworks, fw);
}

void tick_fireworks(uint64_t ticks) {
    // Update each firework.
    for(int i = 0; i < vector_size(fireworks); i++) {
        struct firework * fw = &fireworks[i];
        fw->x += fw->xvel * ticks;
        fw->y += fw->yvel * ticks;

        fw->yvel += ACCEL * ticks;
    }

    for(int i = 0; i < vector_size(fireworks); i++) {
        // Remove dead fireworks.
        struct firework * fw = &fireworks[i];
        if(fw->x < 0 || fw->x > WIN_WIDTH || fw->y < 0 || fw->y > WIN_HEIGHT) {
            vector_erase(fireworks, i);
            i--;
        }
    }

    for(int i = 0; i < vector_size(fireworks); i++) {
        // Generate sparks.
        struct firework * fw = &fireworks[i];
        if(fw->yvel > 0 && fw->isrocket) {
            // Generate a bunch of smaller fireworks going in random directions.
            int number = rand() % 50 + 50;
            for(int j = 0; j < number; j++) {
                struct firework fw2 = { fw->color, 0, 0, fw->x, fw->y };

                // Random velocities.
                fw2.xvel = (rand() % 1000 - 500) / 1000.0f;
                fw2.yvel = (rand() % 1000 - 500) / 1000.0f;
                fw2.xvel /= 5.0f;
                fw2.yvel /= 5.0f;
                fw2.lifetime = rand() % 900 + 900;

                fw2.color.r -= 32;
                fw2.color.g -= 32;
                fw2.color.b -= 32;

                switch(rand() % 3) {
                    case 0: fw2.color.r += rand() % 32; break;
                    case 1: fw2.color.g += rand() % 32; break;
                    case 2: fw2.color.b += rand() % 32; break;
                }

                vector_push_back(fireworks, fw2);
            }

            vector_erase(fireworks, i);
            i--;
        }
    }

    for(int i = 0; i < vector_size(fireworks); i++) {
        // Update the lifetime of each firework.
        struct firework * fw = &fireworks[i];
        if(fw->isrocket == 0) {
            fw->lifetime -= ticks;
            if(fw->lifetime <= 0) {
                vector_erase(fireworks, i);
                i--;
            }
        }
    }

    // Render all fireworks.
    for(int i = 0; i < vector_size(fireworks); i++) {
        struct firework * fw = &fireworks[i];

        // Draw the master stroke:
        SDL_SetTextureColorMod(firework_brush, fw->color.r, fw->color.g, fw->color.b);
        
        SDL_Rect rect = { fw->x - 8, fw->y - 8, 8, 8 };
        SDL_RenderCopy(renderer, firework_brush, NULL, &rect);

        float myx = rect.x, myy = rect.y;

        // Draw the trail: Smaller copies of the brush with increasing Alpha channel value.
        float prevyvel = fw->yvel;
        for(int j = 1; j < 15; j++) {
            SDL_SetTextureColorMod(firework_brush, fw->color.r, fw->color.g, fw->color.b);
            SDL_SetTextureAlphaMod(firework_brush, fw->lifetime / 5 - j * 10);

            fw->yvel -= ACCEL * 35;
            myx -= fw->xvel * 25;
            myy -= fw->yvel * 25;
            rect.x = myx; rect.y = myy;
            SDL_RenderCopy(renderer, firework_brush, NULL, &rect);
        }
        fw->yvel = prevyvel;
    }
}

void load_textures() {
    #define loadas(buf, dest) \
    { \
        SDL_Surface * surface = SDL_LoadBMP_RW(SDL_RWFromConstMem(buf, sizeof(buf)), 1); \
        if(surface == NULL) { \
            abort(); \
        } \
        SDL_SetColorKey(surface, SDL_TRUE, 0xFF00FF); \
        dest = SDL_CreateTextureFromSurface(renderer, surface); \
        SDL_FreeSurface(surface); \
    }

    // Load card faces and backs.
    loadas(back_bmp, card_backs[0]);
    loadas(bfish_bmp, card_backs[1]);
    loadas(bghatch_bmp, card_backs[2]);
    loadas(bkvine_bmp, card_backs[3]);
    loadas(brobot_bmp, card_backs[4]);
    loadas(cfish_bmp, card_backs[5]);
    loadas(ghost_bmp, card_backs[6]);
    loadas(krazkstl_bmp, card_backs[7]);
    loadas(rbhatch_bmp, card_backs[8]);
    loadas(rose_bmp, card_backs[9]);
    loadas(sanflipe_bmp, card_backs[10]);
    loadas(shell_bmp, card_backs[11]);
    loadas(yfish_bmp, card_backs[12]);

    loadas(clk_bmp, card_faces[0][0]);
    loadas(clq_bmp, card_faces[0][1]);
    loadas(clj_bmp, card_faces[0][2]);
    loadas(cl10_bmp, card_faces[0][3]);
    loadas(cl9_bmp, card_faces[0][4]);
    loadas(cl8_bmp, card_faces[0][5]);
    loadas(cl7_bmp, card_faces[0][6]);
    loadas(cl6_bmp, card_faces[0][7]);
    loadas(cl5_bmp, card_faces[0][8]);
    loadas(cl4_bmp, card_faces[0][9]);
    loadas(cl3_bmp, card_faces[0][10]);
    loadas(cl2_bmp, card_faces[0][11]);
    loadas(cla_bmp, card_faces[0][12]);

    loadas(dmk_bmp, card_faces[1][0]);
    loadas(dmq_bmp, card_faces[1][1]);
    loadas(dmj_bmp, card_faces[1][2]);
    loadas(dm10_bmp, card_faces[1][3]);
    loadas(dm9_bmp, card_faces[1][4]);
    loadas(dm8_bmp, card_faces[1][5]);
    loadas(dm7_bmp, card_faces[1][6]);
    loadas(dm6_bmp, card_faces[1][7]);
    loadas(dm5_bmp, card_faces[1][8]);
    loadas(dm4_bmp, card_faces[1][9]);
    loadas(dm3_bmp, card_faces[1][10]);
    loadas(dm2_bmp, card_faces[1][11]);
    loadas(dma_bmp, card_faces[1][12]);

    loadas(htk_bmp, card_faces[2][0]);
    loadas(htq_bmp, card_faces[2][1]);
    loadas(htj_bmp, card_faces[2][2]);
    loadas(ht10_bmp, card_faces[2][3]);
    loadas(ht9_bmp, card_faces[2][4]);
    loadas(ht8_bmp, card_faces[2][5]);
    loadas(ht7_bmp, card_faces[2][6]);
    loadas(ht6_bmp, card_faces[2][7]);
    loadas(ht5_bmp, card_faces[2][8]);
    loadas(ht4_bmp, card_faces[2][9]);
    loadas(ht3_bmp, card_faces[2][10]);
    loadas(ht2_bmp, card_faces[2][11]);
    loadas(hta_bmp, card_faces[2][12]);

    loadas(spk_bmp, card_faces[3][0]);
    loadas(spq_bmp, card_faces[3][1]);
    loadas(spj_bmp, card_faces[3][2]);
    loadas(sp10_bmp, card_faces[3][3]);
    loadas(sp9_bmp, card_faces[3][4]);
    loadas(sp8_bmp, card_faces[3][5]);
    loadas(sp7_bmp, card_faces[3][6]);
    loadas(sp6_bmp, card_faces[3][7]);
    loadas(sp5_bmp, card_faces[3][8]);
    loadas(sp4_bmp, card_faces[3][9]);
    loadas(sp3_bmp, card_faces[3][10]);
    loadas(sp2_bmp, card_faces[3][11]);
    loadas(spa_bmp, card_faces[3][12]);

    loadas(FELT_bmp, background);

    loadas(CARDBACK_bmp, spiderCard);

    loadas(stroke_bmp, firework_brush);

    #undef loadas

    font = TTF_OpenFontRW(SDL_RWFromConstMem(micross_ttf, sizeof(micross_ttf)), 1, 14);
    big_font = TTF_OpenFontRW(SDL_RWFromConstMem(micross_ttf, sizeof(micross_ttf)), 1, 24);
}

int create_window() {
    window = SDL_CreateWindow("Spider Solitaire", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetWindowIcon(window, SDL_LoadBMP_RW(SDL_RWFromConstMem(sol_bmp, sizeof(sol_bmp)), 1));

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        return 1;
    }

    return 0;
}

void render_felt() {
    for(int i = 0; i <= WIN_WIDTH / 64; i++) {
        for(int j = 0; j <= WIN_HEIGHT / 64; j++) {
            SDL_Rect rect;

            rect.x = i * 64;
            rect.y = j * 64;
            rect.w = 64;
            rect.h = 64;

            SDL_RenderCopy(renderer, background, NULL, &rect);
        }
    }
}

void render_card(int x, int y, int suit, int rank) {
    SDL_Rect rect;

    rect.x = x;
    rect.y = y;
    rect.w = CARD_WIDTH;
    rect.h = CARD_HEIGHT;

    SDL_RenderCopy(renderer, card_faces[suit][rank], NULL, &rect);
}

void render_card_back(int x, int y) {
    SDL_Rect rect;

    rect.x = x;
    rect.y = y;
    rect.w = CARD_WIDTH;
    rect.h = CARD_HEIGHT;

    SDL_RenderCopy(renderer, card_backs[game.selected_card_back], NULL, &rect);
}

void render_spider_card(int x, int y) {
    SDL_Rect rect;

    rect.x = x;
    rect.y = y;
    rect.w = CARD_WIDTH;
    rect.h = CARD_HEIGHT;

    SDL_RenderCopy(renderer, spiderCard, NULL, &rect);
}

void render_hollow_card(int x, int y) {
    // Black border 1px filled with darker green to indicate a stack slot.
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = CARD_WIDTH;
    rect.h = CARD_HEIGHT;
    SDL_SetRenderDrawColor(renderer, 0, 0x80, 0, 127);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
}
