#ifndef __TWEEN_H__
#define __TWEEN_H__


#include <corecrt.h>
enum tween_type {
        LINEAR,
        EASE_IN,
        EASE_OUT,
        EASE_IN_OUT
};
struct tween {
	enum tween_type t_type;
        float *var;
        float from, to;
        float duration;
        float _progress;
        _Bool ended;
};

void tween_update(float fdt);
void tween_init(void);

// idx shouldn't be used, because it might change and it depends
// on not fully working arraylist
size_t add_tween(struct tween tween); // returns tween idx
void end_tween(size_t idx);


float easeInSine(float x);
float easeOutSine(float x);
float easeInOutSine(float x);
float easeInCubic(float x);
float easeOutCubic(float x);
float easeInOutCubic(float x);

#endif
