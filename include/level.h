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
	struct arraylist rd_grf_areas;
};

void load_level(struct leveldata data);
void reload_level(void);
void level_control(void);
_Bool level_star_collected(void);

#endif
