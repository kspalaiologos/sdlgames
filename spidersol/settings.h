
#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "ui.h"

#define SETTINGS_WND_HEIGHT 150
#define SETTINGS_WND_WIDTH 258
#define SETTINGS_WND_OFFSET_X (WIN_WIDTH - SETTINGS_WND_WIDTH) / 2
#define SETTINGS_WND_OFFSET_Y (WIN_HEIGHT - SETTINGS_WND_HEIGHT) / 2
#define SETTINGS_CARDS_X_SPACING 10

void settingsHandleRender();
void settingsHandleMouseUp(int x, int y);

#endif
