#include "player.h"
#include "game.h"
#include "graph.h"
#include "engine/tex.h"

#include "engine/physics.h"
#include <raymath.h>
#include <stdio.h>
#include <string.h>

extern struct game game;

static struct player player = { 0 };

static _Bool player_contained(Vector2 p, float *distance)
{
	//printf("p { %f, %f }\n", p.x, p.y);
	const Vector2 v = { player.pos.x - p.x, player.pos.y - p.y };
	*distance = Vector2Length(v);
	return Vector2Length(v) <= player.radius;
}

static _Bool player_collides_with_graph(graph_t graph, Vector2 *point)
{
	// TODO fix this mess later

	const int precision = 10;
	const float px = player.pos.x;
	
	float old_dist = INFINITY;
	float dist;

	for (int x = (int)(px - player.radius) * precision;
	     x <= (int)(px + player.radius) * precision; ++x) {
		const float y =
			-graph((float)x / (float)precision / GRAPH_SCALE);
		const Vector2 p = { (float)x / (float)precision,
				    y * GRAPH_SCALE };
		
		if (player_contained(p, &dist)) {
			if (dist < old_dist) {
				old_dist = dist;
				point->x = p.x;
				point->y = p.y;
			}
			
			//return 1;
		}
	}
	if (old_dist != INFINITY) {
		return 1;
	}
	return 0;
}

_Bool player_collides(Vector2 *point)
{
	// check direct collisions against all graphs
	for (int i = 0; i < game.ngraphs; ++i)
		if (player_collides_with_graph(game.graphs[i], point))
			return 1;
	return 0;
}

void player_init(void)
{
	game.player = &player;

	player.pos = (Vector2){ 0, -256 };
	player.radius = 20.0F;
	player.tex_size = 24.0F;
	player.tint = (Color){ 255, 255, 255, 255 };

	texture_load(&player.tex, "res/img/ball/puffer.png");

	player.body.mass = 1;
	player.body.moment_of_inertia = calculate_circle_inertia(player.radius);
	
}
