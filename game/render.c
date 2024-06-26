#include "engine/render.h"
#include "graph.h"
#include "player.h"
#include "engine/tex.h"
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include <malloc.h>
#include <string.h>

#define OBSTACLE_COLOR ORANGE

extern struct game game;

static struct {
	Color graph_color;
	struct leveldata *leveldata;
} rctx;

Texture2D star_tex;
Texture2D dest_tex;

void render_init(void)
{
	rctx.graph_color = RED;
	rctx.leveldata = malloc(sizeof *rctx.leveldata);

	game.ngraphs = 1;
	game.graphs = malloc(game.ngraphs * sizeof(graph_t));

	texture_load(&star_tex, "res/img/star.png");
	texture_load(&dest_tex, "res/img/destination.png");
}

static void render_obstacle(struct obstacle *obstacle)
{
	Rectangle rect = { .x = obstacle->pos.x,
			   .y = obstacle->pos.y,
			   .width = obstacle->size.x,
			   .height = obstacle->size.y };
	DrawRectanglePro(rect, (Vector2){ rect.width / 2, rect.height / 2 },
			 obstacle->rotation, OBSTACLE_COLOR);
}

static void render_obstacles(void)
{
	const struct arraylist *obstacles = &rctx.leveldata->obstacles;
	for (size_t i = 0; i < arraylist_count(obstacles); ++i)
		render_obstacle(arraylist_get(obstacles, i));
}

static void render_destination(void)
{
	texture_draw(&dest_tex, rctx.leveldata->b, (Vector2){ 40, 40 }, 0,
		     WHITE);
}

static void render_star(void)
{
	texture_draw(&star_tex, rctx.leveldata->star, (Vector2){ 30, 30 }, 0,
		     WHITE);
}

void render(void)
{
	texture_draw(&game.background, game.camera.target,
		     (Vector2){ game.window->screen_w / game.camera.zoom,
				game.window->screen_h / game.camera.zoom },
		     0, WHITE);
	render_graph();

	render_obstacles();
	render_destination();

	if (!level_star_collected())
		render_star();

	const struct player *player = game.player;
	const Vector2 size = { player->tex_size * 2, player->tex_size * 2 };
	DrawCircleV(player->pos, player->radius, BLUE);
	texture_draw(&player->tex, player->pos, size, player->rotation,
		     player->tint);

	DrawCircleV(player->body.collision, 2, ORANGE);
	DrawLineV(player->pos,
		  (Vector2){ player->pos.x + player->body.debug.x,
			     player->pos.y + player->body.debug.y },
		  RED);
}

void render_feed_leveldata(const struct leveldata *data)
{
	memcpy(rctx.leveldata, data, sizeof *data);
}
