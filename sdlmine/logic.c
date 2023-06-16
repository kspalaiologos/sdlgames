
#include "logic.h"

#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#include "main.h"

static int won(void);

unsigned minefield_x = 9, minefield_y = 9, minefield_mines = 10;
char * minefield = NULL;
char * minefield_visible = NULL;

void regenerate_minefield(unsigned x, unsigned y, unsigned mines) {
    srand(time(NULL));

    free(minefield);
    free(minefield_visible);

    minefield = calloc(x * y, 1);
    minefield_visible = calloc(x * y, 1);

    for (unsigned i = 0; i < mines; i++) {
        unsigned mine_x = rand() % x;
        unsigned mine_y = rand() % y;
        if (minefield[mine_y * x + mine_x] == 9) {
            i--;
            continue;
        }
        minefield[mine_y * x + mine_x] = 9;
        if (mine_y > 0 && mine_x > 0 && minefield[(mine_y - 1) * x + (mine_x - 1)] < 9)
            minefield[(mine_y - 1) * x + (mine_x - 1)]++;
        if (mine_y > 0 && minefield[(mine_y - 1) * x + mine_x] < 9) minefield[(mine_y - 1) * x + mine_x]++;
        if (mine_y > 0 && mine_x < x - 1 && minefield[(mine_y - 1) * x + (mine_x + 1)] < 9)
            minefield[(mine_y - 1) * x + (mine_x + 1)]++;
        if (mine_x > 0 && minefield[mine_y * x + (mine_x - 1)] < 9) minefield[mine_y * x + (mine_x - 1)]++;
        if (mine_x < x - 1 && minefield[mine_y * x + (mine_x + 1)] < 9) minefield[mine_y * x + (mine_x + 1)]++;
        if (mine_y < y - 1 && mine_x > 0 && minefield[(mine_y + 1) * x + (mine_x - 1)] < 9)
            minefield[(mine_y + 1) * x + (mine_x - 1)]++;
        if (mine_y < y - 1 && minefield[(mine_y + 1) * x + mine_x] < 9) minefield[(mine_y + 1) * x + mine_x]++;
        if (mine_y < y - 1 && mine_x < x - 1 && minefield[(mine_y + 1) * x + (mine_x + 1)] < 9)
            minefield[(mine_y + 1) * x + (mine_x + 1)]++;
    }

    for (unsigned i = 0; i < x * y; i++) {
        minefield_visible[i] = VIS_COVERED;
    }

    mine_counter = mines;
}

static void flood_uncover(int x, int y) {
    // We are guaranteed that (x,y) is not a mine.
    // Recurse to every tile around (x,y) as long as it's
    //   1) minefield_visible == VIS_COVERED 2) minefield == 0

    minefield_visible[y * minefield_x + x] = VIS_UNCOVERED;

    if (minefield[y * minefield_x + x] == 0) {
        if (x > 0 && y > 0 && minefield_visible[(y - 1) * minefield_x + (x - 1)] == VIS_COVERED &&
            minefield[(y - 1) * minefield_x + (x - 1)] <= 8) {
            minefield_visible[(y - 1) * minefield_x + (x - 1)] = VIS_UNCOVERED;
            flood_uncover(x - 1, y - 1);
        }

        if (y > 0 && minefield_visible[(y - 1) * minefield_x + x] == VIS_COVERED &&
            minefield[(y - 1) * minefield_x + x] <= 8) {
            minefield_visible[(y - 1) * minefield_x + x] = VIS_UNCOVERED;
            flood_uncover(x, y - 1);
        }

        if (x < minefield_x - 1 && y > 0 && minefield_visible[(y - 1) * minefield_x + (x + 1)] == VIS_COVERED &&
            minefield[(y - 1) * minefield_x + (x + 1)] <= 8) {
            minefield_visible[(y - 1) * minefield_x + (x + 1)] = VIS_UNCOVERED;
            flood_uncover(x + 1, y - 1);
        }

        if (x > 0 && minefield_visible[y * minefield_x + (x - 1)] == VIS_COVERED &&
            minefield[y * minefield_x + (x - 1)] <= 8) {
            minefield_visible[y * minefield_x + (x - 1)] = VIS_UNCOVERED;
            flood_uncover(x - 1, y);
        }

        if (x < minefield_x - 1 && y < minefield_y - 1 &&
            minefield_visible[(y + 1) * minefield_x + (x + 1)] == VIS_COVERED &&
            minefield[(y + 1) * minefield_x + (x + 1)] <= 8) {
            minefield_visible[(y + 1) * minefield_x + (x + 1)] = VIS_UNCOVERED;
            flood_uncover(x + 1, y + 1);
        }

        if (x > 0 && y < minefield_y - 1 && minefield_visible[(y + 1) * minefield_x + (x - 1)] == VIS_COVERED &&
            minefield[(y + 1) * minefield_x + (x - 1)] <= 8) {
            minefield_visible[(y + 1) * minefield_x + (x - 1)] = VIS_UNCOVERED;
            flood_uncover(x - 1, y + 1);
        }

        if (y < minefield_y - 1 && minefield_visible[(y + 1) * minefield_x + x] == VIS_COVERED &&
            minefield[(y + 1) * minefield_x + x] <= 8) {
            minefield_visible[(y + 1) * minefield_x + x] = VIS_UNCOVERED;
            flood_uncover(x, y + 1);
        }

        if (x < minefield_x - 1 && minefield_visible[y * minefield_x + (x + 1)] == VIS_COVERED &&
            minefield[y * minefield_x + (x + 1)] <= 8) {
            minefield_visible[y * minefield_x + (x + 1)] = VIS_UNCOVERED;
            flood_uncover(x + 1, y);
        }
    }
}

int mine_counter = 0;

int toggle_flag(int x, int y) {
    int vis = minefield_visible[x + y * minefield_x];
    int min = minefield[x + y * minefield_x];
    if (vis == VIS_COVERED) {
        minefield_visible[x + y * minefield_x] = VIS_FLAG;
        mine_counter--;
        if (mine_counter < 0) mine_counter = 0;
    } else if (vis == VIS_FLAG) {
        minefield_visible[x + y * minefield_x] = VIS_QUESTION;
        mine_counter++;
        if (mine_counter > 999) mine_counter = 999;
    } else if (vis == VIS_QUESTION)
        minefield_visible[x + y * minefield_x] = VIS_COVERED;
    return won();
}

static int won(void) {
    int won = 1;
    for (int i = 0; i < minefield_x * minefield_y; i++) {
        if (minefield[i] == 9 && minefield_visible[i] != VIS_FLAG) {
            won = 0;
            break;
        }
        if (minefield[i] != 9 && minefield_visible[i] == VIS_COVERED) {
            won = 0;
            break;
        }
    }
    return won;
}

int make_move(int x, int y) {
    int vis = minefield_visible[x + y * minefield_x];
    int min = minefield[x + y * minefield_x];
    if (vis != VIS_COVERED) return 0;
    if (min == 9) {
        for (int i = 0; i < minefield_x * minefield_y; i++) {
            if (minefield[i] == 9) minefield_visible[i] = VIS_OTHER_MINE;
            if (minefield_visible[i] == VIS_FLAG && minefield[i] != 9) minefield_visible[i] = VIS_WRONG_FLAG;
        }
        minefield_visible[x + y * minefield_x] = VIS_LOSING_MINE;
        return -1;
    } else {
        flood_uncover(x, y);
        return won();
    }
}

#include <stdio.h>

void load_settings() {
    FILE * in = fopen("sdlmine.dat", "r");
    if (in == NULL) {
        minefield_x = 9;
        minefield_y = 9;
        minefield_mines = 10;
        black_white = 0;
        sounds_enabled = 1;
        save_settings();
    } else {
        fscanf(in, "%d %d %d\n", &minefield_x, &minefield_y, &minefield_mines);
        fscanf(in, "%d %d\n", &black_white, &sounds_enabled);
        fclose(in);
    }
}

void save_settings() {
    FILE * out = fopen("sdlmine.dat", "w");
    fprintf(out, "%d %d %d\n", minefield_x, minefield_y, minefield_mines);
    fprintf(out, "%d %d\n", black_white, sounds_enabled);
    fflush(out);
    fclose(out);
}
