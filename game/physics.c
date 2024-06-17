#include "engine/physics.h"
#include "game.h"
#include "player.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

extern struct game game;

#define PHYSICS_G 130.0F
#define BOUNCE 0.4F
#define FF 0.5F // friction factor

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

	Vector2 f = (Vector2){0, PHYSICS_G};
	f = Vector2Add(f, player->body.friction);
	player->body.force = f;
	// r is the 'arm vector' that goes from the center of mass to the point of force application
	Vector2 r = (Vector2){200, 0};
	player->body.torque = r.x * f.y - r.y * f.x;

	Vector2 linear_acceleration = (Vector2){player->body.force.x / player->body.mass, player->body.force.y / player->body.mass};


	player->body.linear_velocity.x += linear_acceleration.x * fdt;
	player->body.linear_velocity.y += linear_acceleration.y * fdt;

	player->pos.x += player->body.linear_velocity.x * fdt;
	player->pos.y += player->body.linear_velocity.y * fdt;

	float angular_acceleration = player->body.torque / player->body.moment_of_inertia;

	//printf("angular force: %f\n", angular_acceleration);

	player->body.angular_velocity += angular_acceleration * fdt;
	player->rotation += player->body.angular_velocity * fdt;



	Vector2 coll;
	if (player_collides(&coll)) {
		player->body.collision = coll;
		

		const Vector2 hit_distance = {
			player->pos.x - coll.x,
			player->pos.y - coll.y
		};
		
		const Vector2 vel_normal = Vector2Normalize(player->body.linear_velocity);
		
		Vector2 diff = Vector2Subtract(Vector2Scale(Vector2Normalize(hit_distance), player->radius), hit_distance);
		//player->pos = Vector2Add(player->pos,  diff  ); // based on hit_distance
		player->pos = Vector2Add(player->pos,  Vector2Scale(Vector2Negate(vel_normal), Vector2Length(diff))  ); // based on vel_normal
		
		//printf("dist: { %f, %f}\n", player->body.linear_velocity);

		

		//player->body.linear_velocity = (Vector2){
		// 	-player->body.linear_velocity.x * Vector2DotProduct(vel_normal, coll) * BOUNCE,
		// 	-player->body.linear_velocity.y * Vector2DotProduct(vel_normal, coll) * BOUNCE};


		player->body.coll_nor = Vector2Normalize((Vector2){player->pos.x-coll.x, player->pos.y-coll.y});
		Vector2 move = Vector2Scale(Vector2Reflect(player->body.linear_velocity, player->body.coll_nor), BOUNCE);
		player->body.linear_velocity = move;
		
		
		
		//player->body.debug = Vector2Scale(Vector2Subtract(Vector2Scale(Vector2Normalize(hit_distance), player->radius), hit_distance),100);
		player->body.debug = hit_distance;

	}
	//player->body.debug = Vector2Scale(player->body.coll_nor, 10);
	
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
