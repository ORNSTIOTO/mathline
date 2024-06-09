#ifndef __HASH_H__
#define __HASH_H__

#include <string.h>

static unsigned hash_str(const char *s)
{
	unsigned hash = 1315423911;
	unsigned i = 0;

	for (i = 0; i < strlen(s); ++s, ++i)
		hash ^= ((hash << 5) + (*s) + (hash >> 2));

	return hash;
}

#endif
