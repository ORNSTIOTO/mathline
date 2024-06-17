#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <raylib.h>
#include "engine/arraylist.h"

struct leveldata {
	Vector2 a, b;
	struct arraylist *obstacles;
	struct arraylist *rd_grf_areas;
};

void load_level(struct leveldata data);

#endif
