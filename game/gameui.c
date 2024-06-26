#include "game.h"
#include "gameui.h"
#include "level.h"
#include "graph.h"
#include "engine/ui.h"
#include "perfgoals.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

struct ui_object *screenmenu;
struct ui_object *mainmenu;
struct ui_object *levelui;

static void callback_levelbtn_clicked(void *a)
{
	if (!mainmenu->data->visible)
		return;

	struct evtbtn_args *args = a;
	const struct ui_object *btn = args->button;

	const float x =
		btn->data->position.offset.x / (btn->data->size.offset.x + 16);
	const float y =
		btn->data->position.offset.y / (btn->data->size.offset.y + 16);

	const int lvln = (int)x + (int)y * 5 + 2 + 1;

	load_level_num(lvln);
	show_levelui();
}

static void callback_formula_refresh(void *a)
{
	if (!levelui->data->visible)
		return;

	struct evtbox_args *args = a;
	build_fgraph(args->textbox->data->textbox.text.string);
}

static void callback_lvback(void *a)
{
	// if (!levelui->data->visible)
	// 	return;

	level_finish();
	show_mainmenu();
}

static void callback_mm_back_button(void *a)
{
	if (!mainmenu->data->visible)
		return;

	show_screenmenu();
}

static void callback_exit(void *a)
{
	if (!screenmenu->data->visible)
		return;

	game_exit();
}

static void load_screenmenu(void)
{
	struct ui_object *main_screen =
		ui_create(UIC_FRAME, "main_screen", ui_get_root()).object;
	main_screen->data->size = (UDim2){ { 0, 0 }, { 1, 1 } };

	struct ui_object *screenbckg =
		ui_create(UIC_IMAGE, "screenbckg", main_screen).object;
	screenbckg->data->size = (UDim2){ { 1.78F * SCREEN_H, 0 }, { 0, 1 } };
	ui_set_image(screenbckg, "res/img/ui/thumbnail.png");

	struct ui_object *main_side =
		ui_create(UIC_IMAGE, "main_side", main_screen).object;
	
	main_side->data->size = (UDim2){ { 0.6407407407F * SCREEN_H, 0 }, { 0, 1 } };
	main_side->data->position = (UDim2){ { SCREEN_W-487, 0 }, { 0, 0 } };
	ui_set_image(main_side, "res/img/ui/main_side.png");


	// height 400*0.3016877637F ~= 120

	struct ui_object *sm_lvls =
		ui_create(UIC_IMAGEBUTTON, "sm_lvls", main_screen).object;
	sm_lvls->data->imagebutton.img.tint = WHITE;
	sm_lvls->data->anchor = (Vector2){ 0, 1 };
	sm_lvls->data->position = (UDim2){ { -440, 300 }, { 1, 0 } };
	sm_lvls->data->size = (UDim2){ { 400, 400*0.3016877637F }, { 0, 0 } };
	ui_set_image(sm_lvls, "res/img/ui/mm_button.png");
	evt_connect(&sm_lvls->data->imagebutton.btn.events.clicked,
		    callback_lvback);

		struct ui_object *sm_lvls_l = ui_create(UIC_LABEL, "sm_lvls_l", sm_lvls).object;
		sm_lvls_l->data->transparency = 1;
		sm_lvls_l->data->position = (UDim2){ { 140, 40 }, { 0, 0 } };
		sm_lvls_l->data->size = (UDim2){ { 600, 600 }, { 0, 1 } };
		sm_lvls_l->data->label.text.color = WHITE;
		ui_set_text(sm_lvls_l, "Levels");
		ui_set_fonttype(sm_lvls_l, UIF_CRAYON, 50);

	struct ui_object *sm_skins =
		ui_create(UIC_IMAGEBUTTON, "sm_skins", main_screen).object;
	sm_skins->data->imagebutton.img.tint = WHITE;
	sm_skins->data->anchor = (Vector2){ 0, 1 };
	sm_skins->data->position = (UDim2){ { -440, 480 }, { 1, 0 } };
	sm_skins->data->size = (UDim2){ { 400, 400*0.3016877637F }, { 0, 0 } };
	ui_set_image(sm_skins, "res/img/ui/mm_button.png");
	// evt_connect(&sm_skins->data->imagebutton.btn.events.clicked,
	// 	    callback_mm_back_button);

		struct ui_object *sm_skins_l = ui_create(UIC_LABEL, "sm_skins_l", sm_skins).object;
		sm_skins_l->data->transparency = 1;
		sm_skins_l->data->position = (UDim2){ { 150, 40 }, { 0, 0 } };
		sm_skins_l->data->size = (UDim2){ { 600, 600 }, { 0, 1 } };
		sm_skins_l->data->label.text.color = WHITE;
		ui_set_text(sm_skins_l, "Skins");
		ui_set_fonttype(sm_skins_l, UIF_CRAYON, 50);

	struct ui_object *sm_exit =
		ui_create(UIC_IMAGEBUTTON, "sm_exit", main_screen).object;
	sm_exit->data->imagebutton.img.tint = WHITE;
	sm_exit->data->anchor = (Vector2){ 0, 1 };
	sm_exit->data->position = (UDim2){ { -440, 660 }, { 1, 0 } };
	sm_exit->data->size = (UDim2){ { 400, 400*0.3016877637F }, { 0, 0 } };
	ui_set_image(sm_exit, "res/img/ui/mm_button.png");
	evt_connect(&sm_exit->data->imagebutton.btn.events.clicked,
		    callback_exit);

		struct ui_object *sm_exit_l = ui_create(UIC_LABEL, "sm_exit_l", sm_exit).object;
		sm_exit_l->data->transparency = 1;
		sm_exit_l->data->position = (UDim2){ { 160, 40 }, { 0, 0 } };
		sm_exit_l->data->size = (UDim2){ { 600, 600 }, { 0, 1 } };
		sm_exit_l->data->label.text.color = WHITE;
		ui_set_text(sm_exit_l, "Exit");
		ui_set_fonttype(sm_exit_l, UIF_CRAYON, 50);
	

	screenmenu = main_screen;
}

static void load_mainmenu(void)
{
	struct ui_object *bckg =
		ui_create(UIC_IMAGE, "mmbckg", ui_get_root()).object;
	bckg->data->size = (UDim2){ { 0, 0 }, { 1, 1 } };
	ui_set_image(bckg, "res/img/ui/background_lvl.png");

	for (int i = 0; i < 10; ++i) {
		char name[16];
		snprintf(name, 16, "%d:%d", i + 1, i * 11);

		const int x = (i % 5) - 2;
		const int y = i / 5;

		struct ui_object *btn =
			ui_create(UIC_IMAGEBUTTON, name, bckg).object;
		btn->data->imagebutton.img.tint = WHITE;
		btn->data->size = (UDim2){ { 100, 100 }, { 0, 0 } };
		btn->data->anchor = (Vector2){ 0.5F, 0.5F };
		btn->data->position =
			(UDim2){ { (float)x * (btn->data->size.offset.x + 16),
				   (float)y * (btn->data->size.offset.y + 16) },
				 { 0.5F, 0.5F } };
		ui_set_image(btn, "res/img/ui/lvlnumbtn.png");
		evt_connect(&btn->data->imagebutton.btn.events.clicked,
			    callback_levelbtn_clicked);

		char text[16];
		snprintf(text, 16, "%d", i + 1);

		char tname[16];
		snprintf(tname, 16, "%d_uilvlab", i * 11);

		struct ui_object *txt = ui_create(UIC_LABEL, tname, btn).object;
		txt->data->transparency = 1;
		txt->data->position.offset = (Vector2){ 35, 14 };
		txt->data->label.text.color = WHITE;
		ui_set_fonttype(txt, UIF_CRAYON, 80);
		ui_set_text(txt, text);
	}

	struct ui_object *mm_back_button =
		ui_create(UIC_IMAGEBUTTON, "lvmm_back_button", bckg).object;
	mm_back_button->data->imagebutton.img.tint = WHITE;
	mm_back_button->data->anchor = (Vector2){ 0, 1 };
	mm_back_button->data->position = (UDim2){ { 20, -20 }, { 0, 1 } };
	mm_back_button->data->size = (UDim2){ { 70, 60 }, { 0, 0 } };
	ui_set_image(mm_back_button, "res/img/ui/back.png");
	evt_connect(&mm_back_button->data->imagebutton.btn.events.clicked,
		    callback_mm_back_button);

	mainmenu = bckg;
}

static void load_levelui(void)
{
	struct ui_object *canvas =
		ui_create(UIC_CANVAS, "lucanvas", ui_get_root()).object;
	canvas->data->size = (UDim2){ { 0, 0 }, { 1, 1 } };

	struct ui_object *input = ui_create(UIC_IMAGE, "input", canvas).object;
	input->data->position = (UDim2){ { 0, 10 }, { 1, 0 } };
	input->data->size = (UDim2){ { 300, 50 }, { 0, 0 } };
	input->data->anchor = (Vector2){ 1, 0 };
	ui_set_image(input, "res/img/ui/input.png");

	struct ui_object *fxlb = ui_create(UIC_LABEL, "fxlb", input).object;
	fxlb->data->transparency = 1;
	fxlb->data->position = (UDim2){ { 20, 7 }, { 0, 0 } };
	fxlb->data->size = (UDim2){ { 80, 0 }, { 0, 1 } };
	fxlb->data->label.text.color = (Color){ 180, 70, 0, 255 };
	ui_set_text(fxlb, "f(x) =");
	ui_set_fonttype(fxlb, UIF_CRAYON, 30);

	struct ui_object *formula =
		ui_create(UIC_TEXTBOX, "formula", input).object;
	formula->data->transparency = 1;
	formula->data->position = (UDim2){ { 0, 7 }, { 1, 0 } };
	formula->data->size = (UDim2){ { -100, 0 }, { 1, 1 } };
	formula->data->anchor = (Vector2){ 1, 0 };
	formula->data->label.text.color = (Color){ 255, 255, 255, 255 };
	ui_init_text(formula);
	ui_set_fonttype(formula, UIF_CRAYON, 30);

	evt_connect(&formula->data->textbox.box.events.focuslost,
		    callback_formula_refresh);

	struct ui_object *back =
		ui_create(UIC_IMAGEBUTTON, "lvback", canvas).object;
	back->data->imagebutton.img.tint = WHITE;
	back->data->anchor = (Vector2){ 0, 1 };
	back->data->position = (UDim2){ { 20, -20 }, { 0, 1 } };
	back->data->size = (UDim2){ { 70, 60 }, { 0, 0 } };
	ui_set_image(back, "res/img/ui/back.png");	
	evt_connect(&back->data->imagebutton.btn.events.clicked,
		    callback_lvback);

	struct ui_object *calc_back =
		ui_create(UIC_IMAGE, "calc_back", canvas).object;
	calc_back->data->anchor = (Vector2){ 1, 0 };
	calc_back->data->position = (UDim2){ { 0, 10 }, { 1, 0 } };
	calc_back->data->size = (UDim2){ { 310, 310 }, { 0, 0 } };
	ui_set_image(calc_back, "res/img/ui/calc_back.png");

	struct ui_object *pause =
		ui_create(UIC_IMAGEBUTTON, "pause", calc_back).object;
	pause->data->imagebutton.img.tint = WHITE;
	pause->data->anchor = (Vector2){ 1, 0 };
	pause->data->position = (UDim2){ { -10, 70 }, { 1, 0 } };
	pause->data->size = (UDim2){ { 60, 60 }, { 0, 0 } };
	ui_set_image(pause, "res/img/ui/play.png");

	struct ui_object *right_bracket =
		ui_create(UIC_IMAGEBUTTON, "right_bracket", calc_back).object;
	right_bracket->data->imagebutton.img.tint = WHITE;
	right_bracket->data->anchor = (Vector2){ 1, 0 };
	right_bracket->data->position = (UDim2){ { -80, 70 }, { 1, 0 } };
	right_bracket->data->size = (UDim2){ { 60, 60 }, { 0, 0 } };
	ui_set_image(right_bracket, "res/img/ui/calc_button.png");

		struct ui_object *right_bracket_l = ui_create(UIC_LABEL, "right_bracket_l", right_bracket).object;
		right_bracket_l->data->transparency = 1;
		right_bracket_l->data->position = (UDim2){ { 23, 5 }, { 0, 0 } };
		right_bracket_l->data->size = (UDim2){ { 60, 60 }, { 0, 1 } };
		right_bracket_l->data->label.text.color = WHITE;
		ui_set_text(right_bracket_l, ")");
		ui_set_fonttype(right_bracket_l, UIF_CRAYON, 50);

	struct ui_object *left_bracket =
		ui_create(UIC_IMAGEBUTTON, "left_bracket", calc_back).object;
	left_bracket->data->imagebutton.img.tint = WHITE;
	left_bracket->data->anchor = (Vector2){ 1, 0 };
	left_bracket->data->position = (UDim2){ { -150, 70 }, { 1, 0 } };
	left_bracket->data->size = (UDim2){ { 60, 60 }, { 0, 0 } };
	ui_set_image(left_bracket, "res/img/ui/calc_button.png");

		struct ui_object *left_bracket_l = ui_create(UIC_LABEL, "left_bracket_l", left_bracket).object;
		left_bracket_l->data->transparency = 1;
		left_bracket_l->data->position = (UDim2){ { 23, 5 }, { 0, 0 } };
		left_bracket_l->data->size = (UDim2){ { 60, 60 }, { 0, 1 } };
		left_bracket_l->data->label.text.color = WHITE;
		ui_set_text(left_bracket_l, "(");
		ui_set_fonttype(left_bracket_l, UIF_CRAYON, 50);

	struct ui_object *power =
		ui_create(UIC_IMAGEBUTTON, "power", calc_back).object;
	power->data->imagebutton.img.tint = WHITE;
	power->data->anchor = (Vector2){ 1, 0 };
	power->data->position = (UDim2){ { -220, 70 }, { 1, 0 } };
	power->data->size = (UDim2){ { 60, 60 }, { 0, 0 } };
	ui_set_image(power, "res/img/ui/calc_button.png");

		struct ui_object *power_l = ui_create(UIC_LABEL, "power_l", power).object;
		power_l->data->transparency = 1;
		power_l->data->position = (UDim2){ { 25, 5 }, { 0, 0 } };
		power_l->data->size = (UDim2){ { 60, 60 }, { 0, 1 } };
		power_l->data->label.text.color = WHITE;
		ui_set_text(power_l, "^");
		ui_set_fonttype(power_l, UIF_CRAYON, 50);

	struct ui_object *divide =
		ui_create(UIC_IMAGEBUTTON, "divide", calc_back).object;
	divide->data->imagebutton.img.tint = WHITE;
	divide->data->anchor = (Vector2){ 1, 0 };
	divide->data->position = (UDim2){ { -10, 140 }, { 1, 0 } };
	divide->data->size = (UDim2){ { 60, 60 }, { 0, 0 } };
	ui_set_image(divide, "res/img/ui/calc_button.png");

		struct ui_object *divide_l_fnsdjfn = ui_create(UIC_LABEL, "divide_l", divide).object;
		divide_l_fnsdjfn->data->transparency = 1;
		divide_l_fnsdjfn->data->position = (UDim2){ { 23, 5 }, { 0, 0 } };
		divide_l_fnsdjfn->data->size = (UDim2){ { 60, 60 }, { 0, 1 } };
		divide_l_fnsdjfn->data->label.text.color = WHITE;
		ui_set_text(divide_l_fnsdjfn, "/");
		ui_set_fonttype(divide_l_fnsdjfn, UIF_CRAYON, 50);

	struct ui_object *multiplicate =
		ui_create(UIC_IMAGEBUTTON, "multiplicate", calc_back).object;
	multiplicate->data->imagebutton.img.tint = WHITE;
	multiplicate->data->anchor = (Vector2){ 1, 0 };
	multiplicate->data->position = (UDim2){ { -80, 140 }, { 1, 0 } };
	multiplicate->data->size = (UDim2){ { 60, 60 }, { 0, 0 } };
	ui_set_image(multiplicate, "res/img/ui/calc_button.png");

		struct ui_object *multiplicate_l = ui_create(UIC_LABEL, "multiplicate_l", multiplicate).object;
		multiplicate_l->data->transparency = 1;
		multiplicate_l->data->position = (UDim2){ { 21, 3 }, { 0, 0 } };
		multiplicate_l->data->size = (UDim2){ { 60, 60 }, { 0, 1 } };
		multiplicate_l->data->label.text.color = WHITE;
		ui_set_text(multiplicate_l, "*");
		ui_set_fonttype(multiplicate_l, UIF_CRAYON, 50);

	struct ui_object *substract =
		ui_create(UIC_IMAGEBUTTON, "substract", calc_back).object;
	substract->data->imagebutton.img.tint = WHITE;
	substract->data->anchor = (Vector2){ 1, 0 };
	substract->data->position = (UDim2){ { -150, 140 }, { 1, 0 } };
	substract->data->size = (UDim2){ { 60, 60 }, { 0, 0 } };
	ui_set_image(substract, "res/img/ui/calc_button.png");

		struct ui_object *substract_l = ui_create(UIC_LABEL, "substract_l", substract).object;
		substract_l->data->transparency = 1;
		substract_l->data->position = (UDim2){ { 21, 3 }, { 0, 0 } };
		substract_l->data->size = (UDim2){ { 60, 60 }, { 0, 1 } };
		substract_l->data->label.text.color = WHITE;
		ui_set_text(substract_l, "-");
		ui_set_fonttype(substract_l, UIF_CRAYON, 50);
	
	struct ui_object *add =
		ui_create(UIC_IMAGEBUTTON, "add", calc_back).object;
	add->data->imagebutton.img.tint = WHITE;
	add->data->anchor = (Vector2){ 1, 0 };
	add->data->position = (UDim2){ { -220, 140 }, { 1, 0 } };
	add->data->size = (UDim2){ { 60, 60 }, { 0, 0 } };
	ui_set_image(add, "res/img/ui/calc_button.png");

		struct ui_object *add_l = ui_create(UIC_LABEL, "add_l", add).object;
		add_l->data->transparency = 1;
		add_l->data->position = (UDim2){ { 21, 3 }, { 0, 0 } };
		add_l->data->size = (UDim2){ { 60, 60 }, { 0, 1 } };
		add_l->data->label.text.color = WHITE;
		ui_set_text(add_l, "+");
		ui_set_fonttype(add_l, UIF_CRAYON, 50);

	struct ui_object *x =
		ui_create(UIC_IMAGEBUTTON, "x", calc_back).object;
	x->data->imagebutton.img.tint = WHITE;
	x->data->anchor = (Vector2){ 1, 0 };
	x->data->position = (UDim2){ { -220, 210 }, { 1, 0 } };
	x->data->size = (UDim2){ { 60, 60 }, { 0, 0 } };
	ui_set_image(x, "res/img/ui/calc_x_button.png");

		struct ui_object *x_l = ui_create(UIC_LABEL, "x_l", x).object;
		x_l->data->transparency = 1;
		x_l->data->position = (UDim2){ { 21, 3 }, { 0, 0 } };
		x_l->data->size = (UDim2){ { 60, 60 }, { 0, 1 } };
		x_l->data->label.text.color = WHITE;
		ui_set_text(x_l, "x");
		ui_set_fonttype(x_l, UIF_CRAYON, 50);

	struct ui_object *sin =
		ui_create(UIC_IMAGEBUTTON, "sin", calc_back).object;
	sin->data->imagebutton.img.tint = WHITE;
	sin->data->anchor = (Vector2){ 1, 0 };
	sin->data->position = (UDim2){ { -115, 210 }, { 1, 0 } };
	sin->data->size = (UDim2){ { 95, 60 }, { 0, 0 } };
	ui_set_image(sin, "res/img/ui/wide_calc_button.png");

		struct ui_object *sin_l = ui_create(UIC_LABEL, "sin_l", sin).object;
		sin_l->data->transparency = 1;
		sin_l->data->position = (UDim2){ { 21, 3 }, { 0, 0 } };
		sin_l->data->size = (UDim2){ { 60, 60 }, { 0, 1 } };
		sin_l->data->label.text.color = WHITE;
		ui_set_text(sin_l, "sin");
		ui_set_fonttype(sin_l, UIF_CRAYON, 50);

	struct ui_object *abs =
		ui_create(UIC_IMAGEBUTTON, "abs", calc_back).object;
	abs->data->imagebutton.img.tint = WHITE;
	abs->data->anchor = (Vector2){ 1, 0 };
	abs->data->position = (UDim2){ { -10, 210 }, { 1, 0 } };
	abs->data->size = (UDim2){ { 95, 60 }, { 0, 0 } };
	ui_set_image(abs, "res/img/ui/wide_calc_button.png");
	
		struct ui_object *abs_l = ui_create(UIC_LABEL, "abs_l", abs).object;
		abs_l->data->transparency = 1;
		abs_l->data->position = (UDim2){ { 18, 3 }, { 0, 0 } };
		abs_l->data->size = (UDim2){ { 60, 60 }, { 0, 1 } };
		abs_l->data->label.text.color = WHITE;
		ui_set_text(abs_l, "abs");
		ui_set_fonttype(abs_l, UIF_CRAYON, 50);


	levelui = canvas;
}

void load_gameui(void)
{
	load_screenmenu();
	load_mainmenu();
	load_levelui();
}

void show_mainmenu(void)
{
	screenmenu->data->visible = 0;
	mainmenu->data->visible = 1;
	levelui->data->visible = 0;
}

void show_levelui(void)
{
	screenmenu->data->visible = 0;
	mainmenu->data->visible = 0;
	levelui->data->visible = 1;
}

void show_screenmenu(void)
{
	screenmenu->data->visible = 1;
	mainmenu->data->visible = 0;
	levelui->data->visible = 0;
}