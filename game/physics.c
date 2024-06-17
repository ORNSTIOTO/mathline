#include "engine/physics.h"
#include "game.h"
#include "player.h"
#include <raymath.h>
#include <stdio.h>

extern struct game game;

#define PHYSICS_G 130.0F
#define BOUNCE 0.9F

struct {
	_Bool paused;
} physics;

// https://www.toptal.com/game/video-game-physics-part-i-an-introduction-to-rigid-body-dynamics

static void player_update(const float fdt)
{
	if (physics.paused)
		return;

	struct player *player = game.player;

	//	printf("x force: %f\n", player->body.force.x);

	player->body.linear_velocity.x += player->body.linear_accel.x * fdt;
	player->body.linear_velocity.y += PHYSICS_G * fdt;

	//	printf("linear velocity: { %f, %f }    linear accel: { %f, %f }\n",
	//	       player->body.linear_velocity.x, player->body.linear_velocity.y,
	//	       player->body.linear_accel.x, player->body.linear_accel.y);

	player->pos.x += player->body.linear_velocity.x * fdt;
	player->pos.y += player->body.linear_velocity.y * fdt;

	player->body.angular_velocity +=
		player->body.linear_velocity.x * fdt / player->radius;
	player->rotation += player->body.angular_velocity * fdt;

	Vector2 coll;
	if (player_collides(&coll)) {
		// TODO zero out the forces appropriately
		// TODO put the ball back relative to the object hit,
		//   so it doesn't appear clipped or floating.

		Vector2 hit_rel = { coll.x - player->pos.x,
				    coll.y - player->pos.y };
		Vector2 nhit_rel =
			Vector2Scale(Vector2Normalize(hit_rel), player->radius);
		Vector2 d_move = (Vector2){ nhit_rel.x - hit_rel.x,
					    nhit_rel.y - hit_rel.y };

		const Vector2 vel_normal =
			Vector2Normalize(player->body.linear_velocity);
		player->pos.x -= d_move.x * 20;
		player->pos.y -= d_move.y * 20;

		//printf("d distance: { %f }\n", nhit_rel.y);

		//printf("hit_rel: { %f, %f }\nvel_normal: { %f, %f }\n",
		//       hit_rel.x, hit_rel.y, vel_normal.x,
		//       vel_normal.y);

		//for (;;)
		//	;

		Vector2 distance = { player->pos.x - coll.x,
				     player->pos.y - coll.y };
		Vector2 normal = Vector2Normalize(hit_rel);
		Vector2 move = Vector2Scale(
			Vector2Reflect(player->body.linear_velocity, normal),
			BOUNCE);

		player->body.debug.x = normal.x * 10;
		player->body.debug.y = normal.y * 10;

		//player->body.linear_velocity.x = 0;
		//player->body.linear_velocity.y = 0;
		player->body.linear_velocity.x = move.x;
		player->body.linear_velocity.y = move.y;
	}
}

void physics_update(float fdt)
{
	player_update(fdt);
}

void physics_init(void)
{
	physics.paused = 0;
}

void physics_pause(void)
{
	physics.paused = 1;
}

void physics_resume(void)
{
	physics.paused = 0;
}

_Bool physics_is_paused(void)
{
	return physics.paused;
}
