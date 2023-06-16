
#ifndef LOGIC_H
#define LOGIC_H

#define VIS_UNCOVERED 0
#define VIS_QUESTION 1
#define VIS_COVERED 2
#define VIS_FLAG 3
#define VIS_WRONG_FLAG 4
#define VIS_LOSING_MINE 5
#define VIS_OTHER_MINE 6

extern unsigned minefield_x, minefield_y, minefield_mines;
extern char * minefield;
extern char * minefield_visible;
extern int mine_counter;

void regenerate_minefield(unsigned x, unsigned y, unsigned mines);
int toggle_flag(int x, int y);
int make_move(int x, int y);  // 1: won, -1: lost, 0: continue

void load_settings();
void save_settings();

#endif
