#ifndef __UI_H__
#define __UI_H__

#include "engine/evtcon.h"
#include <raylib.h>
#include <stddef.h>

#define UI_MAX_OBJECTS 256

typedef struct {
	Vector2 offset;
	Vector2 scale;
} UDim2;

enum ui_class {
	UIC_NONE,
	UIC_CANVAS,
	UIC_FRAME,
	UIC_LABEL,
	UIC_BUTTON,
	UIC_IMAGE,
	UIC_IMAGEBUTTON,
};

enum ui_font {
	UIF_DEFAULT,
};

enum ui_txtalign_h {
	UI_TXTAH_LEFT,
	UI_TXTAH_CENTER,
	UI_TXTAH_RIGHT,
};

enum ui_txtalign_v {
	UI_TXTAV_TOP,
	UI_TXTAV_CENTER,
	UI_TXTAV_BOTTOM,
};

enum ui_image_scalemode {
	UI_IMGSM_STRETCH,
	UI_IMGSM_KEEPASPECT,
};

struct ui_descriptor;

struct ui_text {
	Color color;
	float transparency;
	enum ui_font font;
	int font_size;
	enum ui_txtalign_h align_horizontal;
	enum ui_txtalign_v align_vertical;
	size_t size; // bytes of text, NOT length
	char *string;
};

struct ui_image {
	Color tint;
	float transparency;
	enum ui_image_scalemode scalemode;
	Texture2D tex;

	float _aspect_ratio;
};

struct ui_button {
	struct {
		struct event hover, hover_exit, lmb_down, lmb_up, clicked;
	} events;
};

struct uie_canvas {};

struct uie_frame {};

struct uie_label {
	struct ui_text text;
};

struct uie_textbox {
	struct ui_text text;
	struct ui_button btn;
};

struct uie_button {
	struct ui_text text;
	struct ui_button btn;
};

struct uie_image {
	struct ui_image img;
};

struct uie_imagebutton {
	struct ui_image img;
	struct ui_button btn;
};

// The descriptor of an object.
//   This is slightly flawed due to the fact that child rendering relies
// on the parent. Ideally, passing just the ui_generic structure to the
// renderer should be enough, but right now we have to pass the entire object.
// However, I could see this logic making sense too. The descriptors are just
// attributes.
//   UPDATE 2024-05-21: This issue can be solved by rendering starting from the
// root-level object all the way down to its descendants. This also eliminates
// the need for specifically creating a z-index property, whereby the engine
// automatically draws the children layered on top of its parent. There will
// likely be no plans of implementing an actual z-index, as the rendering
// order can be controlled through the use of parents (albeit not so cleanly).

struct ui_descriptor {
	enum ui_class class;

	Vector2 anchor;
	UDim2 position, size;
	Color color;
	float transparency; // overrides the color Alpha channel

	_Bool invincible;
	_Bool visible;
	_Bool hidden; // this will not hide children, unlike <visible>

	union {
		struct uie_canvas canvas;
		struct uie_frame frame;
		struct uie_label label;
		struct uie_button button;
		struct uie_image image;
	};

	////////////
	// ENGINE EXCLUSIVE! Do not read or modify.
	struct {
		_Bool has_text;
		_Bool has_image;
		_Bool is_clickable;
	} meta;

	Vector2 _abs_position, _abs_size; // relative to root, not the parent
};

/**
 *   @brief The actual unique object, holding an identity.
 *   @warning This is READ-ONLY! NEVER assign those members directly. Use the
 * appropriate methods, as the engine itself must be aware of any changes.
 * The engine hereby trusts you not to modify those values and under the
 * assumption that the object is in full control of self, does not perform
 * any extra error-checking.
 *   @note Objects do not get drawn unless they have a valid parent.
 */
struct ui_object {
	unsigned id;

	struct ui_object *parent;
	struct ui_object **children;
	size_t child_count;
	size_t child_max;

	struct ui_descriptor *data;
};

struct ui_res {
	int e;
	struct ui_object *object;
};

void ui_set_parent(struct ui_object *obj, struct ui_object *parent);
void ui_set_text(struct ui_object *obj, const char *s);
void ui_set_ftext(struct ui_object *obj, const char *f, ...);
void ui_set_image(struct ui_object *obj, const char *filename);

struct ui_res ui_create(enum ui_class class, const char *name,
			struct ui_object *parent);
int ui_delete(const char *name);

struct ui_object *ui_get(const char *by_name);

void update_stat_counters(void);
void redraw_ui(void);

void ui_init(void);

#endif
