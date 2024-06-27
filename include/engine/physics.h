#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <math.h>
#include <raylib.h>



void physics_update(float fdt);
void physics_init(void);

void physics_pause(void);
void physics_resume(void);
_Bool physics_is_paused(void);

void resolve_collision(Vector2 coll);

static float calculate_circle_inertia(float r)
{
	return M_PI * powf(r, 4.0F) / 4;
}

#endif
