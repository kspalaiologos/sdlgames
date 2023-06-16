
#ifndef _LEADERBOARD_H
#define _LEADERBOARD_H

#include "ui.h"

#define LEADERBOARD_WND_HEIGHT 225
#define LEADERBOARD_WND_WIDTH 290
#define LEADERBOARD_WND_OFFSET_X (WIN_WIDTH - LEADERBOARD_WND_WIDTH) / 2
#define LEADERBOARD_WND_OFFSET_Y (WIN_HEIGHT - LEADERBOARD_WND_HEIGHT) / 2
#define LEADERBOARD_CARDS_X_SPACING 10

void onLeaderboardUpdate();
void leaderboardHandleRender();
void leaderboardHandleMouseUp(int x, int y);

#endif
