#include "graph.h"
#include "game.h"

extern struct game game;

static float scr_border_left(void)
{
	const float zoom = game.camera.zoom;
	return game.camera.target.x / GRAPH_SCALE * zoom -
	       game.window->screen_w / 2;
}

static float scr_border_right(void)
{
	const float zoom = game.camera.zoom;
	return game.camera.target.x / GRAPH_SCALE * zoom +
	       game.window->screen_w / 2;
}

void render_fgraph(float (*f)(float x), Color color)
{
	const float bl = scr_border_left();
	const float br = scr_border_right();
	const float zoom = game.camera.zoom;

	int px = (int)bl * GRAPH_SCALE;
	const int lim = (int)br * GRAPH_SCALE;

	Vector2 prev = {
		(float)px / zoom,
		-(float)f((float)px / zoom) * GRAPH_SCALE,
	};

	for (++px; px <= lim; ++px) {
		const float x = (float)px / GRAPH_SCALE / zoom;
		const float y = -f(x);

		const float cx = x * GRAPH_SCALE;
		const float cy = y * GRAPH_SCALE;

		// draw using rlgl vertices
		const Vector2 current = { cx, cy };
		DrawLineV(prev, current, color);

		prev.x = cx;
		prev.y = cy;
	}
}

void build_fgraph(float (*f)(float x))
{
}
