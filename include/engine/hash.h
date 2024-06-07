#ifndef __HASH_H__
#define __HASH_H__

static unsigned hash_str(const char *s)
{
	unsigned h = 5381;

	while (*s++)
		h = ((h << 5) + h) + *s;

	return h;
}

#endif
