#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <raylib.h>

#define GRAPH_SCALE 30

void graph_init(void);
void render_graph(void);
void build_fgraph(const char *expr);
void render_fgraph_old(float (*f)(float x), Color color);

#endif
