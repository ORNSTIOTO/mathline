#include "level.h"
#include "player.h"
#include "engine/physics.h"
#include "engine/render.h"

#include <stdio.h>

_Bool star_collected = 0;

struct leveldata data;

static void star_collision(void)
{
	star_collected = 1;
}

static void dest_collision(void)
{
	printf("REACHED DESTINATION!\n");
}

void load_level(struct leveldata ldata)
{
	data = ldata;

	star_collected = 0;

	reset_player();
	physics_pause();
	render_feed_leveldata(&data);
	player_move(data.a);
}

void reload_level(void)
{
	load_level(data);
}

void level_control(void)
{
	if (player_collides_with(data.star))
		star_collision();

	if (player_collides_with(data.b))
		dest_collision();
}

_Bool level_star_collected(void)
{
	return star_collected;
}
