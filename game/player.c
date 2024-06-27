#include "player.h"
#include "engine/arraylist.h"
#include "game.h"
#include "graph.h"
#include "engine/tex.h"

#include "engine/physics.h"
#include "level.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>

extern struct game game;

static struct player player = { 0 };

static _Bool player_contained(Vector2 p, float *distance)
{
	//printf("p { %f, %f }\n", p.x, p.y);
	const Vector2 v = { player.pos.x - p.x, player.pos.y - p.y };
	if (distance != NULL) {
		*distance = Vector2Length(v);
	}

	return Vector2Length(v) <= player.radius;
}

static _Bool player_collides_with_graph(Vector2 *point)
{
	// TODO fix this mess later

	const int precision = 10;

	float old_dist = INFINITY;
	float dist;

	size_t points = arraylist_count(&game.graph_points);
	printf("points %i\n", points);
	for (size_t i = 0; i < points; ++i) {
		//	const float x = (float)i - (float)points / 2;
		//	const float *y = arraylist_get(&game.graph_points, i);
		//	const Vector2 p = { (float)x / (float)precision,
		//			    *y * GRAPH_SCALE };

		const Vector2 *p = arraylist_get(&game.graph_points, i);

		if (p == NULL)
			continue;

		Vector2 e = *p;
		e.y *= -1;

		if (player_contained(e, &dist)) {
			if (dist < old_dist) {
				old_dist = dist;
				point->x = e.x;
				point->y = e.y;
			}

			//return 1;
		}
	}
	if (old_dist != INFINITY) {
		return 1;
	}
	return 0;
}

static _Bool player_collides_with_obstacle(struct obstacle ob, Vector2 *point)
{
	const float height = ob.size.y/2;
	Vector2 points[2];
	// if (Vector2Distance(player.pos, ob.pos) < player.radius *2) {
	// 	*point = Vector2Subtract(ob.pos, Vector2Scale(Vector2Subtract(ob.pos, player.pos), 0.5F));
	// 	return 1;}
	// return 0;

	points[0] = Vector2Subtract(ob.pos, Vector2Rotate((Vector2){ob.size.x/2-height,0}, ob.rotation*PI/180.0F) );
	points[1] = Vector2Add(ob.pos, Vector2Rotate((Vector2){ob.size.x/2-height,0}, ob.rotation*PI/180.0F) );

	// 0-1
	Vector2 line_vec = Vector2Subtract(points[1], points[0]);
	Vector2 ballToLineStart = Vector2Subtract(player.pos, points[0]);
	float lineLength = Vector2Length(line_vec);
	Vector2 lineDir = Vector2Normalize(line_vec);
	float projection = Vector2DotProduct(ballToLineStart, lineDir);
	projection = fmaxf(0, fminf(projection, lineLength));
	Vector2 closestPoint = Vector2Add(points[0], Vector2Scale(lineDir, projection));
	Vector2 distToBall = Vector2Subtract(player.pos, closestPoint);
	float dist = Vector2Length(distToBall);
	if (dist <= height + player.radius) {
		*point = Vector2Add(closestPoint, Vector2Scale(Vector2Normalize(distToBall), height));
		return 1;
	}
	return 0;
}

_Bool player_collides(Vector2 *point, int id)
{
	//int frames = (int)arraylist_count(&game.level.obstacles)+1;

	if (player_collides_with_graph(point) && id == 0) {
		//coll_frame++;
		return 1;
	}
	
	/*
	// check direct collisions against all graphs
	for (int i = 0; i < game.ngraphs; ++i) {
		if (player_collides_with_graph(game.graphs[i], point)) return 1;
	}*/
	for (size_t i = 0; i < arraylist_count(&game.level.obstacles); ++i) {
		struct obstacle *ob = arraylist_get(&game.level.obstacles, id+1==(int)i);
		if (player_collides_with_obstacle(*ob, point)) {
			return 1;
		}
	}
	return 0;
}

_Bool player_collides_with(Vector2 p)
{
	return player_contained(p, NULL);
}

void player_init(void)
{
	game.player = &player;

	player.pos = (Vector2){ 0, -256 };
	player.radius = 20.0F;
	player.tex_size = 24.0F;
	player.tint = (Color){ 255, 255, 255, 255 };

	player.body.mass = 1;
	player.body.moment_of_inertia = calculate_circle_inertia(player.radius);
}

void reset_player(void)
{
	player.rotation = 0;
	player.body.linear_velocity = Vector2Zero();
	player.body.angular_velocity = 0;
	player.body.linear_accel = Vector2Zero();
}

void player_set_skin(const char *filename)
{
	UnloadTexture(player.tex);
	texture_load(&player.tex, filename);
}

void player_move(Vector2 to)
{
	player.pos = to;
}
