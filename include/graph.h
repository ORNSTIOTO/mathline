#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <raylib.h>

#define GRAPH_SCALE 30

void render_fgraph(float (*f)(float x), Color color);
void build_fgraph(float (*f)(float x));

#endif
