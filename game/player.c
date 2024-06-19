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

static _Bool player_contained(Vector2 p)
{
	//printf("p { %f, %f }\n", p.x, p.y);
	const Vector2 v = { player.pos.x - p.x, player.pos.y - p.y };
	return Vector2Length(v) <= player.radius;
}

static _Bool player_collides_with_graph(graph_t graph, Vector2 *point)
{
	// TODO fix this mess later

	const int precision = 10;
	const float px = player.pos.x;
	for (int x = (int)(px - player.radius) * precision;
	     x <= (int)(px + player.radius) * precision; ++x) {
		const float y =
			-graph((float)x / (float)precision / GRAPH_SCALE);
		const Vector2 p = { (float)x / (float)precision,
				    y * GRAPH_SCALE };
		if (player_contained(p)) {
			point->x = p.x;
			point->y = p.y;
			return 1;
		}
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

_Bool player_collides_with(Vector2 p)
{
	return player_contained(p);
}

void player_init(void)
{
	game.player = &player;

	player.pos = (Vector2){ 0, -256 };
	player.radius = 20.0F;
	player.tint = (Color){ 255, 255, 255, 255 };

	texture_load(&player.tex, "res/img/ball/puffer.png");
}

void player_move(Vector2 to)
{
	player.pos = to;
}
