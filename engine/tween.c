#include "engine/arraylist.h"
#include "game.h"
#include "engine/tween.h"
#include <math.h>
#include <stdio.h>

extern struct game game;

size_t offset = 0;

/*
(struct tween){
        .t_type = EASE_IN_OUT,
        .var = &game.player->pos.x,
        .to = -100.0F,
        .duration = 5}
*/

void tween_init() {
        game.tweens = arraylist_create(sizeof(struct tween), 1);
}

void tween_update(float fdt) {
        //printf("\e[31msizze, %i\e[0m\n", arraylist_count(&game.tweens));
        for (size_t i = offset; i < arraylist_count(&game.tweens); i++) {
                
                struct tween *tw = arraylist_get(&game.tweens, i);
                
                if (*tw->var == tw->to) {
                        end_tween(i);
                        continue;
                }
                
                tw->_progress += fdt / tw->duration;
                //printf("\e[31mprogress, %f\e[0m\n", tw->_progress);


                float mod = tw->_progress;
                //mod = 1.2F*sqrtf(1- (1.55f*mod-1)*(1.55f*mod-1) );
                //mod = easeInOutSine(tw->_progress);
                //mod = easeOutCubic(mod);

                switch (tw->t_type) {
                        case LINEAR:
                                break;
                        case EASE_IN:
                                mod = easeInCubic(mod);
                                break;
                        case EASE_OUT:
                                mod = easeOutCubic(mod);
                                break;
                        case  EASE_IN_OUT:
                                mod = easeInOutCubic(mod);
                                break;
                        default:
                                break;
                }
                

                *tw->var = tw->from + mod *(tw->to-tw->from);
                
                if (tw->_progress >= 1) {
                        *tw->var = tw->to;
                        end_tween(i);
                }
                
        }
}

size_t add_tween(struct tween tween) {
        if (tween.from == 0) {
                tween.from = *tween.var;
        }
        tween._progress = 0;
        return arraylist_pushback(&game.tweens, &tween); 
}


// https://easings.net/

void end_tween(size_t idx) {
        offset++;
        //arraylist_remove(&game.tweens, idx);
}

float easeInSine(float x) {
        return 1 - cosf((x * PI) / 2);
}

float easeOutSine(float x) {
        return sinf((x * PI) / 2);
}

float easeInOutSine(float x) {
        return -(cosf(PI * x) - 1) / 2;
}

float easeInCubic(float x) {
        return x * x * x;
}

float easeOutCubic(float x) {
        return 1 - powf(1 - x, 3);
}

float easeInOutCubic(float x) {
        return x < 0.5 ? 4 * x * x * x : 1 - powf(-2 * x + 2, 3) / 2;
}

