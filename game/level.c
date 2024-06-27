#include "level.h"
#include "game.h"
#include "player.h"
#include "graph.h"
#include "engine/physics.h"
#include "engine/render.h"
#include "gameui.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

_Bool star_collected = 0;

//struct leveldata data;
extern struct game game;

enum levelfile_idx {
	LFIDX_UNKNOWN,
	LFIDX_FUNC,
	LFIDX_A,
	LFIDX_B,
	LFIDX_STAR,
	LFIDX_OBSTACLES,
};

static void star_collision(void)
{
	star_collected = 1;
}

static void dest_collision(void)
{
	level_finish();
}

static void load_level(struct leveldata ldata)
{
	//data = ldata;
	game.level = ldata;

	star_collected = 0;

	reset_player();
	physics_pause();
	render_feed_leveldata(&game.level);
	player_move(game.level.a);
	build_fgraph(game.level.func);
}

void reload_level(void)
{
	load_level(game.level);
}

void level_control(void)
{
	if (player_collides_with(game.level.star))
		star_collision();

	if (player_collides_with(game.level.b))
		dest_collision();
}

_Bool level_star_collected(void)
{
	return star_collected;
}

void level_finish(void)
{
	physics_pause();
	show_victoryui();
}

struct lpar {
	const char *file;
	size_t idx;
	struct leveldata ldata;
};

static enum levelfile_idx lfidx_from_name(const char *name)
{
	if (strcmp(name, "func") == 0)
		return LFIDX_FUNC;
	if (strcmp(name, "a") == 0)
		return LFIDX_A;
	if (strcmp(name, "b") == 0)
		return LFIDX_B;
	if (strcmp(name, "star") == 0)
		return LFIDX_STAR;
	if (strcmp(name, "obstacles") == 0)
		return LFIDX_OBSTACLES;

	return LFIDX_UNKNOWN;
}

static char raw_advance(struct lpar *lpar)
{
	return lpar->file[lpar->idx++];
}

static char raw_peek(struct lpar *lpar)
{
	return lpar->file[lpar->idx];
}

static void skip_spaces(struct lpar *lpar)
{
	while (raw_peek(lpar) == ' ')
		raw_advance(lpar);
}

static char advance(struct lpar *lpar)
{
	//skip_spaces(lpar);
	return raw_advance(lpar);
}

static char peek(struct lpar *lpar)
{
	//skip_spaces(lpar);
	return raw_peek(lpar);
}

static enum levelfile_idx read_name(struct lpar *lpar)
{
	char buf[16];
	size_t i = 0;

	for (;;) {
		const char c = advance(lpar);

		if (c == 0 || (c < 'a' || c > 'z'))
			break;

		buf[i++] = c;
	}

	buf[i] = 0;

	return lfidx_from_name(buf);
}

static void read_until(struct lpar *lpar, char until, char *buf)
{
	size_t i = 0;

	for (;;) {
		const char c = advance(lpar);

		if (c == 0 || c == '\n' || c == until)
			break;

		buf[i++] = c;
	}

	buf[i] = 0;
}

static float read_number(struct lpar *lpar)
{
	char buf[16];
	size_t i = 0;

	skip_spaces(lpar);

	for (;;) {
		const char c = advance(lpar);

		if (c == 0 || ((c < '0' || c > '9') && c != '.' && c != '-'))
			break;

		buf[i++] = c;
	}

	buf[i] = 0;

	return (float)atof(buf);
}

static void read_func(struct lpar *lpar, char *buf)
{
	read_until(lpar, '\n', buf);
}

static void omit(struct lpar *lpar)
{
	skip_spaces(lpar);
	advance(lpar);
	skip_spaces(lpar);
}

static void read_obstacle(struct lpar *lpar)
{
	const float px = read_number(lpar);
	const float py = read_number(lpar);
	const float sx = read_number(lpar);
	const float sy = read_number(lpar);
	const float rt = read_number(lpar);

	struct obstacle ob = {
		.pos = { px, py },
		.size = { sx, sy },
		.rotation = rt,
	};

	arraylist_pushback(&lpar->ldata.obstacles, &ob);

	skip_spaces(lpar);
}

static void read_obstacles(struct lpar *lpar)
{
	for (;;) {
		const char c = peek(lpar);

		if (c == 0 || c == '\n')
			break;

		read_obstacle(lpar);
	}
}

static void read_pair(struct lpar *lpar)
{
	const enum levelfile_idx name = read_name(lpar);

	switch (name) {
	case LFIDX_FUNC: {
		char func[64];
		read_func(lpar, func);
		strcpy(lpar->ldata.func, func);
		break;
	}
	case LFIDX_A: {
		const float x = read_number(lpar);
		const float y = read_number(lpar);
		lpar->ldata.a = (Vector2){ x, y };
		break;
	}
	case LFIDX_B: {
		const float x = read_number(lpar);
		const float y = read_number(lpar);
		lpar->ldata.b = (Vector2){ x, y };
		break;
	}
	case LFIDX_STAR: {
		const float x = read_number(lpar);
		const float y = read_number(lpar);
		lpar->ldata.star = (Vector2){ x, y };
		break;
	}
	case LFIDX_OBSTACLES: {
		read_obstacles(lpar);
		break;
	}
	case LFIDX_UNKNOWN:
		printf("UNKNOWN PAIR NAME FOUND IN LEVEL FILE.\n");
		break;
	}
}

static int load_level_file(const char *filename)
{
	char *file = LoadFileText(filename);

	if (file == NULL)
		return -1;

	struct lpar lpar = {
		.file = file,
		.idx = 0,
		.ldata = { .obstacles = arraylist_create_preloaded(
				   sizeof(struct obstacle), 8, 1), },
	};

	for (size_t i = 0; i < 5; ++i)
		read_pair(&lpar);

	load_level(lpar.ldata);

	return 0;
}

int load_level_num(int n)
{
	char filename[16];
	snprintf(filename, 16, "res/lvl/%d.lvl", n);

	return load_level_file(filename);
}
