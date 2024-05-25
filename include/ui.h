// NOTE: The following header needs -fms-extensions to compile.

#ifndef __UI_H__
#define __UI_H__

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
};

enum ui_font {
	UIF_DEFAULT,
};

struct ui_text {
	Color color;
	float transparency;
	enum ui_font font;
	int font_size;
	size_t size; // bytes of text, NOT length
	char *string;
};

struct uie_canvas {};

struct uie_frame {};

struct uie_label {
	struct ui_text text;
};

struct uie_button {
	struct ui_text text;
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
	};

	////////////
	// ENGINE EXCLUSIVE! Do not read or modify.
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

struct ui_res ui_create(enum ui_class class, const char *name,
			struct ui_object *parent);
int ui_delete(const char *name);

struct ui_object *ui_get(const char *by_name);

void update_stat_counters(void);
void redraw_ui(void);

void ui_init(void);

#endif
