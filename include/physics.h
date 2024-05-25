#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <math.h>
#include <raylib.h>

struct body {
	float mass;
	Vector2 linear_velocity, linear_accel, force;
	float angular_velocity;
	float moment_of_inertia;
};

void physics_update(float fdt);
void physics_init(void);

static float calculate_circle_inertia(float r)
{
	return M_PI * powf(r, 4.0F) / 4;
}

#endif
