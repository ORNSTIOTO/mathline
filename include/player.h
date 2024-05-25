#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <raylib.h>

struct player {
	Vector2 pos, old_pos;
	float radius, angle;
	Color color;

	struct {
		Vector2 linear_velocity, linear_accel;
		Vector2 force;
		float angular_velocity;
	} body;
};

_Bool player_collides(Vector2 *point);
void player_init(void);

#endif
