#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <raylib.h>

struct player {
	Vector2 pos, old_pos;
	float radius, rotation;

	Texture2D tex;
	float tex_size;
	Color tint;

	struct {
		float mass;
		Vector2 linear_velocity, linear_accel, force, friction;
		float angular_velocity;
		float torque, moment_of_inertia;
		Vector2 collision, coll_nor, debug;
		int on_ground;
	} body;
};

_Bool player_collides(Vector2 *point);
_Bool player_collides_with(Vector2 p);
void player_init(void);
void reset_player(void);

void player_move(Vector2 to);

#endif
