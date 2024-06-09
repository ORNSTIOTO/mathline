#include "engine/tex.h"
#include <string.h>

void texture_load(Texture2D *into, const char *filename)
{
	const Image image = LoadImage(filename);
	const Texture2D tex = LoadTextureFromImage(image);
	UnloadImage(image);
	memcpy(into, &tex, sizeof tex);
}

void texture_draw(const Texture2D *tex, Vector2 pos, Vector2 size,
		  float rotation, Color tint)
{
	const Rectangle src = {
		.x = 0,
		.y = 0,
		.width = (float)tex->width,
		.height = (float)tex->height,
	};
	const Rectangle dest = {
		.x = pos.x,
		.y = pos.y,
		.width = size.x,
		.height = size.y,
	};
	const Vector2 origin = { size.x / 2, size.y / 2 };
	DrawTexturePro(*tex, src, dest, origin, rotation, tint);
}
