#include "game.h"
#include "engine/tex.h"
#include "level.h"
#include "player.h"
#include "gameconfig.h"
#include "graph.h"

#include "engine/render.h"
#include "engine/physics.h"
#include "engine/ui.h"

#include <raylib.h>
#include <raymath.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#define DEBUG_KEYS 0

struct game game = { 0 };

struct {
	float from, to, lerp;
} zoom;

static void setup_camera(void)
{
	game.camera.target = (Vector2){ 0.0F, 0.0F };
	game.camera.rotation = 0.0F;
	game.camera.zoom = 1.0F;

	zoom.from = zoom.to = game.camera.zoom;
}

static void background_init(void)
{
	texture_load(&game.background, "res/img/ui/back1line.png");
}

void game_init(struct window *window)
{
	game.window = window;
	game.tip = malloc(32);
	strcpy(game.tip, "press P to unpause");

	setup_camera();
	physics_init();
	render_init();
	player_init();
	ui_init();
	background_init();

	//build_fgraph("((4x - 1) * 5 + 3 - 8 / (x + 3)) / 2");

	build_fgraph("(1/10000)*x*x*x");

	physics_pause();

	struct arraylist obstacles =
		arraylist_create(sizeof(struct obstacle), 1);
	struct obstacle ob1 = { .pos = (Vector2){ 10, -30 },
				.size = (Vector2){ 100, 45 },
				30 };
	arraylist_pushback(&obstacles, &ob1);
	struct leveldata test = {
		.a = (Vector2){ 0, -300 },
		.b = (Vector2){ 0, -20 },
		.star = (Vector2){ 0, -100 },
		.obstacles = obstacles,
		.rd_grf_areas = (struct arraylist){ 0 },
	};
	load_level(test);
}

static void keyboard_debug(void)
{
	const int c = GetCharPressed();

	switch (c) {
	case 'p':
		if (physics_is_paused())
			physics_resume();
		else
			physics_pause();
		break;
	default:
		break;
	}
}

static void handle_input(void)
{
#if DEBUG_KEYS == 1
	keyboard_debug();
#endif
	ui_resolve_keyboard();

	const Vector2 mouse_delta = GetMouseDelta();
	const float mdx = mouse_delta.x;
	const float mdy = mouse_delta.y;

	// clicking
	ui_resolve_mouse();

	// dragging
	if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && (mdx != 0 || mdy != 0)) {
		//game.camera.target.x -= mdx / game.camera.zoom;
		//game.camera.target.y -= mdy / game.camera.zoom;
	}

	// FIXME debug moving
	if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && (mdx != 0 || mdy != 0)) {
		game.player->pos.x += mdx;
		game.player->pos.y += mdy;
	}
	// FIXME debug moving
	if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) && (mdx != 0 || mdy != 0)) {
		game.player->body.linear_velocity.x += mdx;
		game.player->body.linear_velocity.y += mdy;
	}

	// zooming
	// const float scroll = GetMouseWheelMove();
	// if (scroll > 0) {
	// 	zoom.from = game.camera.zoom;
	// 	zoom.to = zoom.from * (1.0F + ZOOM_DELTA);
	// 	zoom.lerp = 0;
	// } else if (scroll < 0) {
	// 	zoom.from = game.camera.zoom;
	// 	zoom.to = zoom.from * (1.0F - ZOOM_DELTA);
	// 	zoom.lerp = 0;
	// }
}

static void lerp_camera_zoom(float dt)
{
	if (zoom.lerp >= 1.0F) {
		zoom.lerp = 1.0F;
		game.camera.zoom = zoom.to;
		return;
	}

	zoom.lerp = Clamp(zoom.lerp + ZOOM_RATE * dt, 0.0F, 1.0F);
	game.camera.zoom = Lerp(zoom.from, zoom.to, zoom.lerp);
}

static void check_for_lose(void)
{
	if (game.player->pos.y > 700) {
		strcpy(game.tip, "      reloaded level");
		reload_level();
	}
}

static void redraw_game(void)
{
	render();
}

static void window_update(void)
{
	const int w = GetScreenWidth();
	const int h = GetScreenHeight();
	game.window->screen_w = (float)w;
	game.window->screen_h = (float)h;

	game.camera.offset.x = game.window->screen_w / 2.0F;
	game.camera.offset.y = game.window->screen_h / 2.0F;
}

void fixed_update(float fdt)
{
	window_update();
	//if (IsKeyDown(KEY_F)) {
	physics_update(fdt);
	//}
}

void update(void)
{
	level_control();
	check_for_lose();
	handle_input();
	update_stat_counters();
}

void late_update(float dt)
{
	lerp_camera_zoom(dt);
	//game.camera.target.x = game.player->pos.x;
	//game.camera.target.y = game.player->pos.y;
}

void final_update(void)
{
	BeginDrawing();
	ClearBackground(BLACK);

	BeginMode2D(game.camera);
	redraw_game();
	EndMode2D();

	redraw_ui();

	EndDrawing();
}

void process_frame(void)
{
	const float dt = GetFrameTime();

	fixed_update(dt);
	update();
	late_update(dt);
	final_update();
}
