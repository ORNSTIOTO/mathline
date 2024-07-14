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


Texture2D star_tex;
Texture2D dest_tex;

void render_init(void)
{

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

	// DrawCircleV(Vector2Subtract(obstacle->pos, Vector2Rotate((Vector2){obstacle->size.x/2-obstacle->size.y/2,0}, obstacle->rotation*PI/180.0F) ), 10, RED);
	// DrawCircleV(Vector2Add(obstacle->pos, Vector2Rotate((Vector2){obstacle->size.x/2-obstacle->size.y/2,0}, obstacle->rotation*PI/180.0F) ), 10, RED);
}

static void render_obstacles(void)
{
	if (sizeof game.level == 0)
		return;
	//if (rctx.leveldata == NULL)
	//	return;

	const struct arraylist *obstacles = &game.level.obstacles;
	if (obstacles->data == NULL)
		return;

	for (size_t i = 0; i < arraylist_count(obstacles); ++i) {
		struct obstacle *o = arraylist_get(obstacles, i);
		render_obstacle(arraylist_get(obstacles, i));
	}
}

static void render_destination(void)
{
	texture_draw(&dest_tex, game.level.b, (Vector2){ 40, 40 }, 0,
		     WHITE);
}

static void render_star(void)
{
	texture_draw(&star_tex, game.level.star, (Vector2){ 30, 30 }, 0,
			WHITE);
}

static void render_background(void)
{
	texture_draw(&game.background.main, game.camera.target,
		(Vector2){ game.window->screen_w / game.camera.zoom,
			game.window->screen_h / game.camera.zoom },
		0, WHITE);

	texture_draw(&game.background.arr_right, (Vector2){game.camera.target.x,0},
		(Vector2){ game.window->screen_w / game.camera.zoom,
			game.window->screen_h / game.camera.zoom },
		0, WHITE);

	texture_draw(&game.background.arr_up, (Vector2){0,game.camera.target.y},
		(Vector2){ game.window->screen_w / game.camera.zoom,
			game.window->screen_h / game.camera.zoom },
		0, WHITE);
}

void render(void)
{
	render_background();
	render_graph();

	render_obstacles();
	render_destination();

	if (!level_star_collected())
		render_star();

	const struct player *player = game.player;
	const Vector2 size = { player->tex_size * 2, player->tex_size * 2 };
	// DrawCircleV(player->pos, player->radius, BLUE);
	texture_draw(&player->tex, player->pos, size, player->rotation,
		     player->tint);

	// DrawCircleV(player->body.collision, 2, GREEN);
	// DrawLineV(player->pos,
	// 	  (Vector2){ player->pos.x + player->body.debug.x,
	// 		     player->pos.y + player->body.debug.y },
	// 	  RED);
}
