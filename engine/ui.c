#include "engine/ui.h"
#include "engine/tex.h"
#include "engine/hash.h"
#include "engine/arraylist.h"
#include "graph.h"
#include "game.h"
#include "player.h"
#include "errno.h"
#include <raylib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>

#define CHILD_COUNT_INCREMENT 32
#define TEXT_DEFAULT_CAPACITY 128

#define FONTNAME_DEBUG "res/fnt/debug_ibmvga9x16.ttf"
#define FONTNAME_CRAYON "res/fnt/dkcrayon-reg.otf"

extern struct game game;

const struct ui_res UIRES_BAD_ID = { -1, 0 };

struct {
	size_t count;
	struct ui_object *list;
	// list[0] is always the root!
} ui_objects;

struct {
	size_t nmemb;
	struct ui_object *buttons[256]; // TODO dynamic
	struct ui_object *down_on; // used for the "Clicked" event
	struct ui_object *focused; // textbox, used for "FocusLost" event
} click_area;

struct textures {
	struct arraylist fonts; // <struct ui_font>
	struct arraylist images; // <Texture2D>  - NOT IMPL
} textures;

const struct ui_descriptor desc_default_attributes = {
	.class = UIC_NONE,
	.anchor = (Vector2){ 0, 0 },
	.position = (UDim2){ { 0, 0 }, { 0, 0 } },
	.size = (UDim2){ { 200, 100 }, { 0, 0 } },
	.color = (Color){ 192, 192, 192, 0 },
	.transparency = 0,
	
	.invincible = 0,
	.visible = 1,
	.hidden = 0,

	._meta = {
		.valid = 0,
		.has_image = 0,
		.has_text = 0,
		.is_clickable = 0,
	},

	._abs_position = (Vector2){ 0, 0 },
	._abs_size = (Vector2){ 0, 0 },
};

const struct uie_canvas canvas_default_attributes = {};

const struct uie_frame frame_default_attributes = {};

const struct uie_label label_default_attributes = {
	.text.color = BLACK,
	.text.transparency = 0,
	.text.font = {
		.type = UIF_DEFAULT,
		.data = NULL,
		.size = 16,
		.spacing_px = 0,
		.linespacing_px = 0,
	},
	.text.autowrap = 1,
	.text.overflow = 0,
	.text.size = 0,
	.text.string = NULL,
};

const struct uie_button button_default_attributes = {
	.text.color = BLACK,
	.text.transparency = 0,
	.text.font = {
		.type = UIF_DEFAULT,
		.data = NULL,
		.size = 16,
		.spacing_px = 2,
		.linespacing_px = 0,
	},
	.text.autowrap = 1,
	.text.overflow = 0,
	.text.size = 0,
	.text.string = NULL,
};

const struct uie_textbox textbox_default_attributes = {
	.text.color = BLACK,
	.text.transparency = 0,
	.text.font = {
		.type = UIF_DEFAULT,
		.data = NULL,
		.size = 16,
		.spacing_px = 2,
		.linespacing_px = 0,
	},
	.text.autowrap = 1,
	.text.overflow = 0,
	.text.size = 0,
	.text.string = NULL,

	.box.focusmode = UI_BOXFM_CLICK,
	.box.cursor = 0,
};

const struct uie_image image_default_attributes = {
	.img.tint = (Color){ 255, 255, 255, 255 },
	.img.transparency = 0,
	.img.tex = { 0 },
};

const struct uie_imagebutton imagebutton_default_attributes = {
	.img.tint = (Color){ 255, 255, 255, 255 },
	.img.transparency = 0,
	.img.tex = { 0 },
};

struct ui_object *ui_get_root(void)
{
	return &ui_objects.list[0];
}

static unsigned calculate_id(const char *name)
{
	return hash_str(name);
}

static Vector2 get_screen_dim(void)
{
	const int w = GetScreenWidth();
	const int h = GetScreenHeight();
	return (Vector2){ (float)w, (float)h };
}

static _Bool id_exists(unsigned id)
{
	for (size_t i = 0; i < ui_objects.count; ++i)
		if (ui_objects.list[i].id == id)
			return 1;
	return 0;
}

static struct ui_object *add_entry(unsigned id, struct ui_descriptor *data)
{
	struct ui_object *this = &ui_objects.list[ui_objects.count++];
	this->id = id;
	this->child_count = 0;
	this->child_max = CHILD_COUNT_INCREMENT;
	this->children = malloc(this->child_max * sizeof(struct ui_object *));
	this->parent = NULL;
	this->data = data;
	return this;
}

static void remove_entry_at(size_t i)
{
	const size_t osize = sizeof(struct ui_object);
	struct ui_object *obj = &ui_objects.list[i];

	ui_objects.count--;

	const size_t count = ui_objects.count;

	if (i == count) {
		memset(obj, 0, osize);
		return;
	}

	memmove(obj - osize, obj, (count - i) * osize);
	memset(&ui_objects.list[count], 0, osize);
}

static void remove_entry(unsigned id)
{
	for (size_t i = 0; i < ui_objects.count; ++i) {
		struct ui_object *obj = &ui_objects.list[i];

		if (obj->id != id)
			continue;

		remove_entry_at(i);
		break;
	}
}

static int find_entry(unsigned id)
{
	for (size_t i = 0; i < ui_objects.count; ++i)
		if (ui_objects.list[i].id == id)
			return (int)i;
	return -1;
}

static unsigned calculate_font_id(const char *fntname, size_t fntsize)
{
	char *dup = strdup(fntname);
	dup[0] = (char)fntsize;
	const unsigned id = calculate_id(dup);
	free(dup);
	return id;
}

static Font ui_rawfont_create(const char *fntname, int fnt_size)
{
	const Font font = LoadFontEx(fntname, fnt_size, 0, 256);
	return font;
}

static struct ui_font ui_font_create(const char *fntname, float fntsize)
{
	const Font font = ui_rawfont_create(fntname, (int)fntsize);
	Font *fontp = malloc(sizeof font);
	memcpy(fontp, &font, sizeof font);

	const struct ui_font fontdata = {
		.id = calculate_font_id(fntname, (size_t)fntsize),
		.type = UIF_RESOURCE,
		.data = fontp,
		.size = fntsize,
		.spacing_px = 2,
	};
	return fontdata;
}

static struct ui_font *ui_font_obtain(const char *fntname, float fntsize,
				      _Bool *created)
{
	struct arraylist *fntlist = &textures.fonts;

	const unsigned id = calculate_font_id(fntname, (size_t)fntsize);
	const size_t idx = arraylist_id_locate(fntlist, id);
	if (idx == -1U) {
		const struct ui_font font = ui_font_create(fntname, fntsize);
		const size_t push_idx = arraylist_pushback(fntlist, &font);

		*created = 1;
		return arraylist_get(fntlist, push_idx);
	}

	struct ui_font *font = arraylist_get(fntlist, idx);

	*created = 0;
	return font;
}

static void register_child(struct ui_object *parent, struct ui_object *child)
{
	// TODO Add error checking for child_count and dynamic allocation.
	parent->children[parent->child_count++] = child;
}

void ui_set_parent(struct ui_object *obj, struct ui_object *parent)
{
	if (obj == NULL || parent == NULL)
		return;

	obj->parent = parent;

	// TODO Update this to remove children, too.
	register_child(parent, obj);
}

void ui_init_text(struct ui_object *obj)
{
	struct ui_text *text = &obj->data->label.text;
	text->string = calloc(TEXT_DEFAULT_CAPACITY, 1);
	text->size = 0;
	text->capacity = TEXT_DEFAULT_CAPACITY;
}

void ui_set_text(struct ui_object *obj, const char *s)
{
	if (obj == NULL || s == NULL)
		return;

	const size_t len = strlen(s) + 1;
	struct ui_text *text = &obj->data->label.text;

	if (text->string != NULL)
		free(text->string);

	text->size = len;
	text->string = malloc(len);
	memcpy(text->string, s, len);
}

void ui_set_ftext(struct ui_object *obj, const char *f, ...)
{
	if (obj == NULL || f == NULL)
		return;

	va_list args;
	va_start(args, f);

	// Determine the length of the formatted string
	va_list args_copy;
	va_copy(args_copy, args);
	int len = vsnprintf(NULL, 0, f, args_copy);
	va_end(args_copy);

	if (len < 0) {
		va_end(args);
		return;
	}

	len++;

	struct ui_text *text = &obj->data->label.text;

	// FIXME Efficiency

	if (text->string != NULL)
		free(text->string);

	text->size = len;
	text->string = malloc(len);
	if (text->string == NULL) {
		va_end(args);
		return;
	}

	vsnprintf(text->string, len, f, args);
	va_end(args);
}

static void unload_font(struct ui_font *font)
{
	if (font->type != UIF_RESOURCE)
		return;

	struct arraylist *fonts = &textures.fonts;
	const size_t loc = arraylist_id_locate(fonts, font->id);
	if (font->data != NULL && loc != -1U) {
		UnloadFont(*font->data);
		free(font->data);
		arraylist_remove(fonts, loc);
	}
}

void ui_set_font(struct ui_object *obj, const char *fntname, float fntsize)
{
	if (obj == NULL || fntname == NULL)
		return;

	_Bool created;
	struct ui_font *newfont = ui_font_obtain(fntname, fntsize, &created);
	newfont->size = (float)fntsize;

	struct ui_font *font = &obj->data->label.text.font;

	if (created)
		unload_font(font);

	memcpy(font, newfont, sizeof *newfont);
	font->type = UIF_RESOURCE;
}

void ui_set_fontsize(struct ui_object *obj, float fntsize)
{
	// TODO not finished
	struct ui_font *font = &obj->data->label.text.font;
	font->size = fntsize;
}

void ui_set_fonttype(struct ui_object *obj, enum ui_font_type ft, float fntsize)
{
	if (ft == UIF_RESOURCE || ft == UIF_DEFAULT)
		return;

	const char *fntname = ft == UIF_CRAYON ? FONTNAME_CRAYON :
			      ft == UIF_DEBUG  ? FONTNAME_DEBUG :
						 NULL;
	ui_set_font(obj, fntname, fntsize);
}

void ui_set_image(struct ui_object *obj, const char *filename)
{
	if (obj == NULL || filename == NULL)
		return;

	struct ui_image *img = &obj->data->image.img;

	if (img->tex.id != 0)
		UnloadTexture(img->tex);

	texture_load(&img->tex, filename);
}

static void set_default_class_attributes(struct ui_descriptor *descriptor)
{
	const void *list;
	size_t size;
	void *dest;

	switch (descriptor->class) {
	case UIC_CANVAS:
		list = &canvas_default_attributes;
		size = sizeof canvas_default_attributes;
		dest = &descriptor->canvas;
		break;
	case UIC_FRAME:
		list = &frame_default_attributes;
		size = sizeof frame_default_attributes;
		dest = &descriptor->frame;
		break;
	case UIC_LABEL:
		list = &label_default_attributes;
		size = sizeof label_default_attributes;
		dest = &descriptor->label;
		break;
	case UIC_BUTTON:
		list = &button_default_attributes;
		size = sizeof button_default_attributes;
		dest = &descriptor->button;
		break;
	case UIC_TEXTBOX:
		list = &textbox_default_attributes;
		size = sizeof textbox_default_attributes;
		dest = &descriptor->textbox;
		break;
	case UIC_IMAGE:
		list = &image_default_attributes;
		size = sizeof image_default_attributes;
		dest = &descriptor->image;
		break;
	case UIC_IMAGEBUTTON:
		list = &imagebutton_default_attributes;
		size = sizeof imagebutton_default_attributes;
		dest = &descriptor->imagebutton;
	default:
		return;
	}

	memcpy(dest, list, size);
}

static void set_default_attributes(struct ui_descriptor *descriptor,
				   enum ui_class class)
{
	memcpy(descriptor, &desc_default_attributes, sizeof *descriptor);
	descriptor->class = class;

	set_default_class_attributes(descriptor);
	descriptor->_meta.valid = 1;
}

static void setup_button_handlers(struct ui_object *button)
{
	// TODO Register click area
	click_area.buttons[click_area.nmemb++] = button;
}

static void initialize_descriptor(struct ui_descriptor *descriptor,
				  enum ui_class class)
{
	set_default_attributes(descriptor, class);
}

static void initialize_object(struct ui_object *object, enum ui_class class)
{
	struct ui_descriptor *data = object->data;

	switch (data->class) {
	case UIC_BUTTON:
	case UIC_IMAGEBUTTON:
	case UIC_TEXTBOX:
		setup_button_handlers(object);
		break;
	default:
		break;
	}
}

static struct ui_res __int_ui_create(enum ui_class class, const char *name,
				     struct ui_object *parent,
				     struct ui_descriptor description,
				     _Bool valid_desc)
{
	const unsigned id = calculate_id(name);
	if (id_exists(id))
		return UIRES_BAD_ID;

	size_t data_size = sizeof(struct ui_descriptor);

	struct ui_descriptor *descriptor = calloc(1, data_size);
	struct ui_object *obj = add_entry(id, descriptor);

	set_default_attributes(descriptor, class);
	initialize_object(obj, class);
	ui_set_parent(obj, parent);

	return (struct ui_res){ 0, obj };
}

struct ui_res ui_create_ext(enum ui_class class, const char *name,
			    struct ui_object *parent,
			    struct ui_descriptor description)
{
	return __int_ui_create(class, name, parent, description, 1);
}

struct ui_res ui_create(enum ui_class class, const char *name,
			struct ui_object *parent)
{
	struct ui_descriptor description = {};
	return __int_ui_create(class, name, parent, description, 0);
}

static void free_class_data(struct ui_descriptor *descriptor)
{
	switch (descriptor->class) {
	case UIC_LABEL:
		free(descriptor->label.text.string);
		break;
	case UIC_IMAGE:
		//free(descriptor->image.img.data);
		break;
	case UIC_BUTTON:
		free(descriptor->button.text.string);
		break;
	case UIC_TEXTBOX:
		free(descriptor->textbox.text.string);
		break;
	default:
		break;
	}
}

static void ui_free(struct ui_object *obj)
{
	for (size_t i = 0; i < obj->child_count; ++i)
		ui_free(obj->children[i]);

	free_class_data(obj->data);
	//free(obj->children);
	free(obj->data);

	remove_entry(obj->id);
}

int ui_delete(const char *name)
{
	const unsigned id = calculate_id(name);
	const int idx = find_entry(id);

	if (idx < 0)
		return -ENOENT;

	struct ui_object *obj = &ui_objects.list[idx];
	struct ui_descriptor *data = obj->data;

	data->_meta.valid = 0;

	if (data->invincible)
		return -EINVAL;

	ui_free(obj);

	return 0;
}

struct ui_object *ui_get(const char *by_name)
{
	const int idx = find_entry(calculate_id(by_name));
	if (idx < 0)
		return NULL;

	return &ui_objects.list[idx];
}

////////////
// DRAWING METHODS BELOW!
//
//   TODO Perhaps make a new file for specifically drawing out of descriptors?
//   TODO By the way since you're seeing this, we need panic methods too for
// the entire game.

//   TODO The issue with recalculate_absolute_size() and _position functions
// is that they are being updated just before rendering. So those absolute
// values are behind by 1 frame and reading them as a user can be useless.
// Try updating them in real time, such as by implementing set_position()
// and set_size() functions. Though, things will get slightly inconsistent.
//   Alternatively, we do not support reading those values by the user, and
// instead implement get_absolute_position() and get_absolute_size() functions
// that they can use on the objects instead. This is more preferable, as it
// avoids unnecessary calculations and does not reduce consistency as much.

static void recalculate_absolute_position(struct ui_descriptor *subject,
					  const struct ui_descriptor *parent)
{
	subject->_abs_position.x =
		parent->_abs_position.x +
		parent->_abs_size.x * subject->position.scale.x +
		subject->position.offset.x -
		subject->_abs_size.x * subject->anchor.x;
	subject->_abs_position.y =
		parent->_abs_position.y +
		parent->_abs_size.y * subject->position.scale.y +
		subject->position.offset.y -
		subject->_abs_size.y * subject->anchor.y;
}

static void recalculate_absolute_size(struct ui_descriptor *subject,
				      const struct ui_descriptor *parent)
{
	subject->_abs_size.x = parent->_abs_size.x * subject->size.scale.x +
			       subject->size.offset.x;
	subject->_abs_size.y = parent->_abs_size.y * subject->size.scale.y +
			       subject->size.offset.y;
}

static void fix_image_position(Vector2 *pos, Vector2 size)
{
	// Fix position to account for origin
	pos->x += size.x / 2;
	pos->y += size.y / 2;
}

static Color fix_color(Color color, float transparency)
{
	return (Color){ color.r, color.g, color.b,
			0xFF - (char)(transparency * 0xFF) };
}

static void draw_rect(const struct ui_descriptor *data)
{
	const Color color = fix_color(data->color, data->transparency);
	DrawRectangleV(data->_abs_position, data->_abs_size, color);
}

static void draw_text(const struct ui_text *text, Vector2 pos)
{
	const _Bool use_default = text->font.type == UIF_DEFAULT;
	const Font font = use_default ? GetFontDefault() : *text->font.data;
	const Color color = fix_color(text->color, text->transparency);
	DrawTextEx(font, text->string, pos, text->font.size,
		   text->font.spacing_px, color);
}

static void draw_bound_text(const struct ui_text *text, Vector2 position,
			    Vector2 bounds, Color tint)
{
	const Font font = *text->font.data;
	const float fsize = text->font.size;
	const float scale_factor = fsize / (float)font.baseSize;
	const float spacing = text->font.spacing_px;
	const char *str = text->string;

	const _Bool aw = text->autowrap;
	const _Bool of = text->overflow;

	// TODO: Create this arraylist just once and do not destroy it.
	struct arraylist codepoints =
		arraylist_create_preloaded(sizeof(int), 64, 0);

	float word_width = 0;

	float x = 0;
	float y = 0;
	for (;;) {
		int codepoint_size;
		const int codepoint = GetCodepoint(str++, &codepoint_size);
		const GlyphInfo glyinfo = GetGlyphInfo(font, codepoint);

		const _Bool is_ws = codepoint == ' ';
		const _Bool is_nl = codepoint == '\n';
		const _Bool is_nul = codepoint == 0;
		const _Bool is_split = is_ws || is_nl || is_nul;

		const float codepoint_width =
			(float)glyinfo.advanceX * scale_factor;
		const float codepoint_height =
			fsize + text->font.linespacing_px;

		if (is_split) {
draw_word:;
			const Vector2 pos = { position.x + x, position.y + y };
			const int ccount = (int)arraylist_count(&codepoints);
			DrawTextCodepoints(font, codepoints.data, ccount, pos,
					   fsize, spacing, tint);

			arraylist_clear(&codepoints);

			if (!is_split)
				goto ret_newline;

			//   NOTE: DrawTextCodepoints() does not draw
			// whitespaces, since it is not a glyph. Therefore,
			// we are just going to manipulate the word width.
			word_width += codepoint_width;

			if (is_nl) {
				x = 0;
				y += codepoint_height;
				word_width = 0;
				continue;
			}

			if (is_nul)
				break;

			x += word_width;
			word_width = 0;

			continue;
		}

		const float this_width = codepoint_width + spacing;

		if (x + word_width + this_width > bounds.x) {
			if (aw && x != 0) {
				x = 0;
				y += codepoint_height;
			} else if (!of)
				break;
		}

		if (aw && word_width + this_width > bounds.x) {
			goto draw_word;
ret_newline: // Return point from goto

			word_width = 0;
			x = 0;
			y += codepoint_height;
		}

		word_width += this_width;

		if (!is_ws && !is_nl && !is_nul)
			arraylist_pushback(&codepoints, &codepoint);
	}

	arraylist_destroy(&codepoints);
}

/*check if ui element is visible depending on its ancestors*/
static _Bool ui_visible(const struct ui_object *obj)
{
	struct ui_object ins = *obj;
	// TODO might be dangerous
	while (1) {
		if (!ins.data->visible) {
			return 0;
		}
		if (ins.parent == NULL) {
			return 1;
		}
		ins = *ins.parent;
	}
}

static void draw_image(const struct ui_image *img, Vector2 pos, Vector2 size,
		       float transparency)
{
	const Color tint = fix_color(img->tint, transparency);
	fix_image_position(&pos, size);
	texture_draw(&img->tex, pos, size, 0, tint);
}

static void ui_draw_frame(const struct ui_descriptor *data)
{
	draw_rect(data);
}

static void ui_draw_label(const struct ui_descriptor *data)
{
	draw_rect(data);
	draw_bound_text(&data->label.text, data->_abs_position,
			data->_abs_size, data->label.text.color);
}

static void ui_draw_button(const struct ui_descriptor *data)
{
	draw_rect(data);
	draw_bound_text(&data->button.text, data->_abs_position,
			data->_abs_size, data->label.text.color);
}

static void ui_draw_image(const struct ui_descriptor *data)
{
	draw_image(&data->image.img, data->_abs_position, data->_abs_size,
		   data->transparency);
}

static void ui_draw_imagebutton(const struct ui_descriptor *data)
{
	ui_draw_image(data);
}

static void ui_draw_textbox(const struct ui_descriptor *data)
{
	const _Bool foc = data->textbox.box.focused;
	const Color tint = (Color){ 20, 20, 120, 255 };
	draw_rect(data);
	draw_bound_text(&data->textbox.text, data->_abs_position,
			data->_abs_size, foc ? tint : data->textbox.text.color);

	//if (data->textbox.box.focused)
	// 	draw_cursor(data->_abs_position, );
}

static void ui_draw_single(const struct ui_descriptor *data)
{
	switch (data->class) {
	case UIC_FRAME:
		ui_draw_frame(data);
		break;
	case UIC_LABEL:
		ui_draw_label(data);
		break;
	case UIC_BUTTON:
		ui_draw_button(data);
		break;
	case UIC_IMAGE:
		ui_draw_image(data);
		break;
	case UIC_TEXTBOX:
		ui_draw_textbox(data);
		break;
	case UIC_IMAGEBUTTON:
		ui_draw_imagebutton(data);
		break;
	default:
		break;
	}
}

static void ui_draw_tree(const struct ui_object *obj,
			 const struct ui_descriptor *parent)
{
	if (!obj->data->visible)
		return;

	if (!ui_visible(obj))
		return;

	if (parent != NULL && !obj->data->hidden) {
		recalculate_absolute_position(obj->data, parent);
		recalculate_absolute_size(obj->data, parent);
		ui_draw_single(obj->data);
	}

	for (size_t i = 0; i < obj->child_count; ++i)
		ui_draw_tree(obj->children[i], obj->data);
}

void redraw_ui(void)
{
	const struct ui_object *root = ui_get_root();
	if (root == NULL)
		return;

	//   Recalculate root's size according to the window size, allowing
	// UI elements to scale dynamically.
	root->data->size.offset = root->data->_abs_size = get_screen_dim();

	ui_draw_tree(root, NULL);
}

static _Bool contained_within(Vector2 point, Vector2 pos, Vector2 size)
{
	return point.x >= pos.x && point.x <= pos.x + size.x &&
	       point.y >= pos.y && point.y <= pos.y + size.y;
}

static struct ui_object *ui_find_hovered_button(void)
{
	const Vector2 mp = GetMousePosition();
	for (size_t i = 0; i < click_area.nmemb; ++i) {
		struct ui_object *button = click_area.buttons[i];
		const struct ui_descriptor *data = button->data;
		if (contained_within(mp, data->_abs_position,
				     data->_abs_size) &&
		    ui_visible(button)) {
			return button;
		}
	}

	return NULL;
}

static void lose_focus(void)
{
	struct evtbox_args args = { .textbox = click_area.focused };
	evt_fire(&click_area.focused->data->textbox.box.events.focuslost,
		 &args);
	click_area.focused->data->textbox.box.focused = 0;
	click_area.focused = NULL;
}

static void gain_focus(struct ui_object *textbox)
{
	struct evtbox_args args = { .textbox = textbox };
	evt_fire(&textbox->data->textbox.box.events.focused, &args);
	click_area.focused = textbox;
	textbox->data->textbox.box.focused = 1;
}

static void button_lmbdown(struct ui_object *button, struct ui_button *btn)
{
	struct evtbtn_args args = {};

	click_area.down_on = args.button = button;
	evt_fire(&btn->events.lmb_down, &args);
}

static void textbox_lmbdown(struct ui_object *textbox)
{
	gain_focus(textbox);
}

static void handle_lmbdown(struct ui_object *on)
{
	if (click_area.focused != NULL)
		lose_focus();

	if (on == NULL)
		return;

	switch (on->data->class) {
	case UIC_BUTTON:
		button_lmbdown(on, &on->data->button.btn);
		break;
	case UIC_IMAGEBUTTON:
		button_lmbdown(on, &on->data->imagebutton.btn);
		break;
	case UIC_TEXTBOX:
		textbox_lmbdown(on);
		break;
	default:
		break;
	}
}

static void button_lmbup(struct ui_object *button, struct ui_button *btn)
{
	struct evtbtn_args args = {};
	const _Bool hovers = button == click_area.down_on;

	args.button = click_area.down_on;

	if (button != NULL)
		evt_fire(&button->data->button.btn.events.lmb_up, &args);
	if (button == click_area.down_on && btn != NULL)
		evt_fire(&btn->events.clicked, &args);
}

static void handle_lmbup(struct ui_object *on)
{
	if (on == NULL) {
		button_lmbup(NULL, NULL);
		return;
	}

	switch (on->data->class) {
	case UIC_BUTTON:
		button_lmbup(on, &on->data->button.btn);
		break;
	case UIC_IMAGEBUTTON:
		button_lmbup(on, &on->data->imagebutton.btn);
		break;
	default:
		break;
	}
}

void ui_resolve_mouse(void)
{
	const _Bool lmb_down = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
	const _Bool lmb_up = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

	if (click_area.down_on == NULL && !lmb_down)
		click_area.down_on = NULL;

	struct ui_object *down_on = click_area.down_on;

	if (lmb_down) {
		struct ui_object *button = ui_find_hovered_button();
		handle_lmbdown(button);

		return;
	}

	if (lmb_up) {
		struct ui_object *button = ui_find_hovered_button();
		handle_lmbup(button);

		click_area.down_on = NULL;
		return;
	}
}

static void text_resize_to(struct ui_text *text, size_t to)
{
	void *tmp = realloc(text->string, to);
	if (tmp == NULL)
		return;

	text->string = tmp;
	text->string[text->size] = 0;
	text->capacity = to;
}

static void text_resize(struct ui_text *text)
{
	if (text->capacity <= TEXT_DEFAULT_CAPACITY / 2)
		text_resize_to(text, TEXT_DEFAULT_CAPACITY);
	else
		text_resize_to(text, text->capacity * 2);
}

static void text_write(struct ui_text *text, char c, size_t at)
{
	if (text->size + sizeof c + 1 >= text->capacity)
		text_resize(text);

	char *s = text->string;

	if (at == text->size) {
		s[at] = c;
		s[at + 1] = 0;
		text->size++;
		return;
	}
}

static void text_erase(struct ui_text *text, size_t at)
{
	if (at > text->size)
		return;

	char *s = text->string;
	s[at] = 0;
	text->size--;
}

void textbox_write(struct ui_object *textbox, char c)
{
	struct uie_textbox *tbox = &textbox->data->textbox;
	struct ui_text *text = &tbox->text;
	text_write(text, c, tbox->box.cursor++);
}

static void textbox_backspace(struct ui_object *textbox)
{
	struct uie_textbox *tbox = &textbox->data->textbox;
	struct ui_text *text = &tbox->text;
	if (tbox->box.cursor > 0)
		text_erase(text, --tbox->box.cursor);
}

static void process_textbox_input(struct ui_object *textbox, int c)
{
	if (c > 0)
		textbox_write(textbox, (char)c);

	if (IsKeyPressed(KEY_BACKSPACE))
		textbox_backspace(textbox);
	if (IsKeyPressed(KEY_ENTER)) {
		lose_focus();
		gain_focus(textbox);
	}
}

void ui_resolve_keyboard(int c)
{
	struct ui_object *focused = click_area.focused;
	if (focused == NULL)
		return;

	process_textbox_input(focused, c);
}

static void ui_create_root(void)
{
	struct ui_object *root = ui_create(UIC_CANVAS, "root", NULL).object;
	root->data->size.offset = root->data->_abs_size = get_screen_dim();
}

static void ui_setup_clickareas(void)
{
	click_area.nmemb = 0;
	click_area.down_on = NULL;
}

static void ui_setup_objlist(void)
{
	ui_objects.count = 0;
	ui_objects.list = malloc(sizeof *ui_objects.list * UI_MAX_OBJECTS);
}

static void ui_setup_texture_buckets(void)
{
	struct ui_font font;
	size_t font_id_off = (uintptr_t)&font - (uintptr_t)&font.id;
	size_t font_id_len = sizeof(font.id);

	Texture2D tex;
	size_t tex_id_off = (uintptr_t)&tex - (uintptr_t)&tex.id;
	size_t tex_id_len = sizeof(tex.id);

	textures.fonts = arraylist_create(sizeof font, 1);
	arraylist_provide_mapping(
		&textures.fonts,
		(struct arraylist_map){ .id.offset = font_id_off,
					.id.length = font_id_len });

	textures.images = arraylist_create(sizeof tex, 1);
	arraylist_provide_mapping(
		&textures.images,
		(struct arraylist_map){ .id.offset = tex_id_off,
					.id.length = tex_id_len });
}

static void formula_box_refresh(void *a)
{
	struct evtbox_args *args = a;
	build_fgraph(args->textbox->data->textbox.text.string);
}

void ui_init(void)
{
	ui_setup_objlist();
	ui_setup_clickareas();
	ui_setup_texture_buckets();
	ui_create_root();

	struct ui_object *root = ui_get_root();

	const Color stat_color = RAYWHITE;
	const float stat_font_size = 20;

	struct ui_object *title = ui_create(UIC_LABEL, "title", root).object;
	title->data->position.offset = (Vector2){ 0, 0 };
	title->data->size = (UDim2){ { 900, 260 }, { 0, 0 } };
	title->data->transparency = 1;
	title->data->label.text.color = DARKBLUE;
	title->data->label.text.autowrap = 1;
	title->data->label.text.overflow = 1;
	ui_set_text(title, "FUNctions");
	ui_set_fonttype(title, UIF_CRAYON, 120);

	struct ui_object *tips = ui_create(UIC_LABEL, "tips", root).object;
	tips->data->position = (UDim2){ { 0 - 280, 15 }, { 1, 0 } };
	tips->data->size = (UDim2){ { 240, 260 }, { 0, 0 } };
	tips->data->anchor = (Vector2){ 1, 0 };
	tips->data->transparency = 1;
	tips->data->label.text.color = WHITE;
	tips->data->label.text.autowrap = 1;
	tips->data->label.text.overflow = 1;
	ui_set_text(tips, game.tip);
	ui_set_fonttype(tips, UIF_CRAYON, 30);

	//	struct ui_object *formula =
	//		ui_create(UIC_TEXTBOX, "formula", root).object;
	//	formula->data->position = (UDim2){ { 0, 100 }, { 1, 0 } };
	//	formula->data->size = (UDim2){ { 300, 100 }, { 0, 0 } };
	//	formula->data->anchor = (Vector2){ 1, 0 };
	//	ui_init_text(formula);
	//	ui_set_fonttype(formula, UIF_CRAYON, 30);
	//
	//	evt_connect(&formula->data->textbox.box.events.focuslost,
	//		    formula_box_refresh);

	/*//#pragma region lvl_ui
	struct ui_object *lvl1 = ui_create(UIC_BUTTON, "lvl1", root).object;
	lvl1->data->position = (UDim2){ { 0,-55 }, { .2f,.5f } };
	lvl1->data->size = (UDim2){ { 101,110 }, { 0,0 } };
	lvl1->data->transparency = 1;
	ui_set_text(lvl1, " ");
	ui_set_fonttype(lvl1, UIF_DEBUG, 1);

	struct event *e_lvl1 = &lvl1->data->button.btn.events.clicked;
	evt_connect(e_lvl1, lvl_button_click);

	struct ui_object *lvl1_i = ui_create(UIC_IMAGE, "lvl1_i", root).object;
	lvl1_i->data->position = (UDim2){ { 0,-55 }, { .2f,.5f } };
	lvl1_i->data->size = (UDim2){ { 101,110 }, { 0,0 } };
	ui_set_image(lvl1_i, "res/img/ui/btn_s.png");

	struct ui_object *lvl1_stars = ui_create(UIC_IMAGE, "lvl1_stars", root).object;
	lvl1_stars->data->position = (UDim2){ { 0,45 }, { .2f,.5f } };
	lvl1_stars->data->size = (UDim2){ { 201/2,105/2 }, { 0,0 } };
	ui_set_image(lvl1_stars, "res/img/ui/stars0.png");

	//#pragma endregion*/

	// struct event *e_c = &button->data->button.btn.events.clicked;
	// struct event *e_d = &button->data->button.btn.events.lmb_down;
	// struct event *e_u = &button->data->button.btn.events.lmb_up;

	// evt_connect(e_c, testbtn_event_function_c);
	// evt_connect(e_d, testbtn_event_function_d);
	// evt_connect(e_u, testbtn_event_function_u);

	// struct ui_object *textbox = ui_create(UIC_TEXTBOX, "box", root).object;
	// textbox->data->position.offset = (Vector2){ 500, 500 };
	// textbox->data->textbox.text.autowrap = 1;
	// textbox->data->textbox.text.overflow = 1;
	// ui_init_text(textbox);
	// ui_set_fonttype(textbox, UIF_DEBUG, stat_font_size);

	// struct event *e_fd = &textbox->data->textbox.box.events.focused;
	// struct event *e_fl = &textbox->data->textbox.box.events.focuslost;

	// evt_connect(e_fd, testbox_event_function_fd);
	// evt_connect(e_fl, testbox_event_function_fl);
}

void update_stat_counters(void)
{
}
