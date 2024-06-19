#include "engine/render.h"
#include "graph.h"
#include "player.h"
#include "engine/tex.h"
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include <malloc.h>
#include <time.h>

extern struct game game;

static struct {
	Color graph_color;
} rctx;

static float __f(float x)
{
	return x;
	//return sinf(powf(x, x)) * x;
	return sinf(x);
}

static float __g(float x)
{
	return atanf(x);
}

void render_init(void)
{
	rctx.graph_color = RED;

	game.ngraphs = 1;
	game.graphs = malloc(game.ngraphs * sizeof(graph_t));

	game.graphs[0] = __f;
	//game.graphs[1] = __g;
}

void render(void)
{
	texture_draw(&game.background, game.camera.target, (Vector2){game.window->screen_w / game.camera.zoom, game.window->screen_h / game.camera.zoom}, 0,
		     WHITE);

	for (int i = 0; i < game.ngraphs; ++i)
		render_fgraph(game.graphs[i], i % 2 == 0 ? RED : BLUE);

	const struct player *player = game.player;
	const Vector2 size = { player->tex_size * 2, player->tex_size * 2 };
	DrawCircleV(player->pos, player->radius, BLUE);
	texture_draw(&player->tex, player->pos, size, player->rotation,
		     player->tint);

	DrawCircleV(player->body.collision, 2, ORANGE);
	DrawLineV(player->pos, (Vector2){
		player->pos.x + player->body.debug.x,
		player->pos.y + player->body.debug.y
		}, RED);
}
