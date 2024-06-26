#include "gameui.h"
#include "level.h"
#include "graph.h"
#include "engine/ui.h"
#include <stdio.h>
#include <stdlib.h>

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
	if (!levelui->data->visible)
		return;

	level_finish();
	show_mainmenu();
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

	mainmenu = bckg;
}

static void load_levelui(void)
{
	struct ui_object *canvas =
		ui_create(UIC_CANVAS, "lucanvas", ui_get_root()).object;
	canvas->data->size = (UDim2){ { 0, 0 }, { 1, 1 } };

	struct ui_object *input = ui_create(UIC_IMAGE, "input", canvas).object;
	input->data->position = (UDim2){ { 0, 100 }, { 1, 0 } };
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

	levelui = canvas;
}

void load_gameui(void)
{
	load_mainmenu();
	load_levelui();
}

void show_mainmenu(void)
{
	mainmenu->data->visible = 1;
	levelui->data->visible = 0;
}

void show_levelui(void)
{
	mainmenu->data->visible = 0;
	levelui->data->visible = 1;
}
