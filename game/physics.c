#include "engine/physics.h"
#include "engine/arraylist.h"
#include "game.h"
#include "player.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

extern struct game game;

#define PHYSICS_G 130.0F
#define BOUNCE 0.1F
#define FF 0.99F // friction factor or smth

#define V2right (Vector2){1, 0}

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

	Vector2 f = (Vector2){ 0, PHYSICS_G };
	//f = Vector2Add(f, player->body.friction);
	player->body.force = f;
	// r is the 'arm vector' that goes from the center of mass to the point of force application
	Vector2 r = (Vector2){ 200, 0 };
	//player->body.torque = r.x * f.y - r.y * f.x;
	player->body.torque = player->body.collision.x * player->body.linear_velocity.y - player->body.collision.y * player->body.linear_velocity.x;

	Vector2 linear_acceleration =
		(Vector2){ player->body.force.x / player->body.mass,
			   player->body.force.y / player->body.mass };

	player->body.linear_velocity.x += linear_acceleration.x * fdt;
	player->body.linear_velocity.y += linear_acceleration.y * fdt;


	player->pos.x += player->body.linear_velocity.x * fdt;
	player->pos.y += player->body.linear_velocity.y * fdt;

	float angular_acceleration =
		player->body.torque / player->body.moment_of_inertia;

	//printf("angular force: %f\n", angular_acceleration);

	player->body.angular_velocity += angular_acceleration * fdt;
	player->rotation += player->body.angular_velocity * fdt;

	player->body.friction = (Vector2){ 0, 0 };
	
	Vector2 coll;
	int collides = 0;
	for (int c = 0; c < (int)arraylist_count(&game.level.obstacles)+1;c++) {
	if (player_collides(&coll, c)) {
		player->body.on_ground++;
		resolve_collision(coll);
		collides += 1;
	} else {
		
	}
	}
	if(collides >1) {
		player->body.linear_velocity = Vector2Scale(player->body.linear_velocity, 0.5f);
	}
	if (!collides){
	player->body.on_ground = 0;}
	//player->body.debug = Vector2Scale(player->body.coll_nor, 10);
	player->old_pos = player->pos;
}

void resolve_collision(Vector2 coll)
{
	struct player *player = game.player;

	player->body.collision = coll;
		
	printf("co: %i\n",player->body.on_ground);
		const Vector2 hit_distance = { player->pos.x - coll.x,
					       player->pos.y - coll.y };

		Vector2 slope = Vector2Rotate(player->body.coll_nor, 90);
		if (slope.y > 0)
			slope = Vector2Negate(slope);

		const Vector2 vel_normal =
			Vector2Normalize(player->body.linear_velocity);

		Vector2 diff = Vector2Subtract(
			Vector2Scale(Vector2Normalize(hit_distance),
				     player->radius),
			hit_distance);

		Vector2 c;
		player->pos = Vector2Add(player->pos,  diff  ); // based on hit_distance
		//player->pos = Vector2Add(player->pos,  Vector2Scale(Vector2Negate(vel_normal), Vector2Length(diff))  ); // based on vel_normal
		// for (int i = 0; i < 20; i++) { // based on coll_nor
		// 	player->pos = Vector2Add(player->pos, player->body.coll_nor);
		// 	if (!player_collides(&c))
		// 		break;
		// }
		//player->pos =
		//	Vector2Add(player->pos, player->body.coll_nor);

		//printf("dist: { %f, %f}\n", player->body.linear_velocity);

		//player->body.linear_velocity = (Vector2){
		// 	-player->body.linear_velocity.x * Vector2DotProduct(vel_normal, coll) * BOUNCE,
		// 	-player->body.linear_velocity.y * Vector2DotProduct(vel_normal, coll) * BOUNCE};

		//player->body.friction = Vector2Scale(slope, Vector2Length(player->body.linear_velocity));
		//player->body.friction = Vector2Rotate((Vector2){0,PHYSICS_G}, Vector2Angle(V2right,slope));



		player->body.coll_nor = Vector2Normalize((Vector2){
			player->pos.x - coll.x, player->pos.y - coll.y });

		Vector2 move = player->body.linear_velocity;
		Vector2 slide = (Vector2){ 0, 0 };
		//move = Vector2Reflect(player->body.linear_velocity,
		//		player->body.coll_nor);

		if (player->body.on_ground == 1) {
			move = Vector2Scale(move, BOUNCE);
		} else {
			move = Vector2Scale(move, FF);
			move = Vector2Scale(move, 1 + 1/Vector2Length(move));
			
			Vector2 delta = Vector2Subtract(player->old_pos, player->pos);
			float dir = Vector3Normalize(Vector3CrossProduct((Vector3){delta.x,delta.y,0}, (Vector3){hit_distance.x,hit_distance.y,0})).z;
			player->rotation += dir * Vector2Length(delta)/player->radius * 180.0F/PI;
		}
		//move = Vector2Rotate((Vector2){0,PHYSICS_G}, Vector2Angle(V2right,slope));
		//slide = Vector2Scale(player->body.linear_velocity, fabsf(Vector2DotProduct(Vector2Normalize(player->body.linear_velocity), slope)) * FF);

		//move = Vector2Add(move, Vector2Rotate((Vector2){0,PHYSICS_G}, Vector2Angle(V2right,slope)));
		//move = Vector2Add(move, slide);
		player->body.linear_velocity = move;
		if (1) {
			//printf("move: { %f, %f}\n", Vector2DotProduct(player->body.linear_velocity, slope),0);
		}

		float a = player->body.mass * PHYSICS_G *
				  sinf(Vector2Angle(V2right, slope)) -
			  player->body.mass * PHYSICS_G * FF *
				  cosf(Vector2Angle(V2right, slope));

		player->body.friction = Vector2Rotate(
			(Vector2){ a, 0 }, Vector2Angle(V2right, slope));
		//player->body.friction = coll;

		//player->body.friction = Vector2DotProduct(player->body.linear_velocity, Vector2Rotate(coll, 90));

		//player->body.debug = Vector2Scale(Vector2Subtract(Vector2Scale(Vector2Normalize(hit_distance), player->radius), hit_distance),100);
		//player->body.debug = Vector2Scale(player->body.coll_nor, 100);
		//player->body.angular_velocity /= 10;
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
