#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <raylib.h>
#include "engine/arraylist.h"

struct obstacle {
	Vector2 pos, size;
	float rotation;
};

struct leveldata {
	Vector2 a, b, star;
	struct arraylist obstacles;
};

int load_level_num(int n);
void reload_level(void);
void level_control(void);
_Bool level_star_collected(void);

void level_finish(void);

#endif
