#ifndef __TEX_H__
#define __TEX_H__

#include <raylib.h>

void texture_load(Texture2D *into, const char *filename);
void texture_draw(const Texture2D *tex, Vector2 pos, Vector2 size,
		  float rotation, Color tint);

#endif
