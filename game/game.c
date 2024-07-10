#include "game.h"
#include "engine/tex.h"
#include "level.h"
#include "player.h"
#include "gameconfig.h"
#include "gameui.h"
#include "graph.h"

#include "engine/render.h"
#include "engine/physics.h"
#include "engine/ui.h"

#include <raylib.h>
#include <raymath.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
	texture_load(&game.background.main, "res/img/ui/back1.png");
	texture_load(&game.background.arr_right, "res/img/ui/arrow_right.png");
	texture_load(&game.background.arr_up, "res/img/ui/arrow_up.png");
}

void game_init(struct window *window)
{
	game.window = window;
	game.tip = malloc(32);
	strcpy(game.tip, "press P to pause\npress R to restart");

	setup_camera();
	physics_init();
	graph_init();
	render_init();
	player_init();
	ui_init();
	background_init();

	physics_pause();

	//build_fgraph("x*sin(x)+(2-x)");

	load_gameui();
	show_screenmenu();
}

static void keyboard_debug(int c)
{
	switch (c) {
	case 'p':
		if (physics_is_paused())
			physics_resume();
		else
			physics_pause();
		break;
	case 'r':
		reload_level();
		break;
	case 'f':
		ToggleFullscreen();
		break;
	default:
		break;
	}
}

static void handle_input(void)
{
	const int c = GetCharPressed();

	keyboard_debug(c);
	ui_resolve_keyboard(c);

	const Vector2 mouse_delta = GetMouseDelta();
	const float mdx = mouse_delta.x;
	const float mdy = mouse_delta.y;

	// clicking
	ui_resolve_mouse();

	// dragging
	// if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && (mdx != 0 || mdy != 0)) {
	// 	game.camera.target.x -= mdx / game.camera.zoom;
	// 	game.camera.target.y -= mdy / game.camera.zoom;
	// }

	// FIXME debug moving
	// if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && (mdx != 0 || mdy != 0)) {
	// 	game.player->pos.x += mdx;
	// 	game.player->pos.y += mdy;
	// }
	// FIXME debug moving
	// if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON) && (mdx != 0 || mdy != 0)) {
	// 	game.player->body.linear_velocity.x += mdx;
	// 	game.player->body.linear_velocity.y += mdy;
	// }

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
	highlight_skin_sel_button();
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

int game_exit(void)
{
	CloseWindow();
	exit(0);
}
