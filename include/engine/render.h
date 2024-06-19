#ifndef __RENDER_H__
#define __RENDER_H__

#include "game.h"
#include "level.h"

void render_init(void);
void render(void);
void render_feed_leveldata(const struct leveldata *data);

#endif
