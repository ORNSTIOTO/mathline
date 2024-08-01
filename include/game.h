#ifndef __GAME_H__
#define __GAME_H__

#include "engine/window.h"
#include "level.h"
#include <engine/arraylist.h>
#include <raylib.h>

#define LEVELS 10

typedef float (*graph_t)(float);

struct game {
	struct window *window;
	struct arraylist tweens;
	struct Camera2D camera;
	struct player *player;
	graph_t *graphs;
	int ngraphs;
	char *tip;
	struct leveldata level;

	struct arraylist graph_points;

	struct {
		Texture2D main;
		Texture2D arr_right;
		Texture2D arr_up;
	} background;

	char game_progress[LEVELS]; // not a string, it's read as numbers
};

void game_init(struct window *window);

// fixed_update() -> update() -> late_update() -> final_update()

void fixed_update(float fdt); // physics calculations
void update(void); // misc
void late_update(float dt); // misc, called after update()
void final_update(void); // rendering

void process_frame(void);

int game_exit(void);



void save_progress(void);
void read_progress(void);

#endif
