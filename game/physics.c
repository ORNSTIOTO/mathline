#include "engine/physics.h"
#include "game.h"
#include "player.h"
#include <raymath.h>
#include <stdio.h>

extern struct game game;

#define PHYSICS_G 130.0F

// https://www.toptal.com/game/video-game-physics-part-i-an-introduction-to-rigid-body-dynamics

static void player_update(const float fdt)
{
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

		const Vector2 hit_distance = {
			fabsf(player->pos.x - coll.x) - player->radius,
			fabsf(player->pos.y - coll.y) - player->radius
		};
		const Vector2 vel_normal =
			Vector2Normalize(player->body.linear_velocity);
		player->pos.x -= vel_normal.x * hit_distance.x;
		player->pos.y -= vel_normal.y * hit_distance.y;

		//printf("hit_distance: { %f, %f }\nvel_normal: { %f, %f }\n",
		//       hit_distance.x, hit_distance.y, vel_normal.x,
		//       vel_normal.y);

		//for (;;)
		//	;

		Vector2 distance = { player->pos.x - coll.x,
				     player->pos.y - coll.y };
		Vector2 normal = Vector2Normalize(coll);
		Vector2 move =
			Vector2Reflect(player->body.linear_velocity, normal);

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
}
