#include "engine/arraylist.h"
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#define MAX(x, y) x > y ? x : y

#define MINIMUM_LIST_SIZE 4
#define MEMB_ALIGNMENT_BYTES 8

static __attribute__((noreturn)) void __int_arraylist_panic(const char *msg)
{
	printf("arraylist panic: %s\n", msg);
	exit(0);
}

static size_t calc_alignment(size_t dsize)
{
	const size_t a = MEMB_ALIGNMENT_BYTES;
	return (dsize + a - 1) / a * a;
}

static _Bool full(struct arraylist *list)
{
	return list->idx + 1 >= list->maxmemb;
}

// Assumes that maxmemb is no less than nmemb.
static size_t get_unused(struct arraylist *list)
{
	return list->maxmemb - list->nmemb;
}

static void compress_area(struct arraylist *list, size_t idx, size_t n)
{
	memmove(list->data + idx, list->data + idx + n, list->size - (idx + n));
	memset(list->data, 0, list->size - n);
}

static size_t compress(struct arraylist *list)
{
	if (list->idx == list->nmemb)
		return 0;

	void *data = list->data;
	const void *zero = list->zero;
	const size_t esize = list->esize;

	size_t idx = 0;
	size_t n = 0;
	size_t blocks = 0;
	_Bool found = 0;

	for (size_t i = 0; i < list->maxmemb; ++i) {
		const _Bool equ = memcmp(zero, data + i * esize, esize) == 0;

		if (!equ && found) {
			compress_area(list, idx * esize, n * esize);
			blocks++;
			idx = n = 0;
			continue;
		}

		if (equ && !found) {
			idx = i;
			n = 1;
			continue;
		}

		if (equ && found) {
			n++;
			continue;
		}
	}

	blocks += n;
	list->idx -= blocks;

	return blocks;
}

static void resize(struct arraylist *list, size_t maxmemb)
{
	const size_t size = maxmemb * list->esize;
	void *tmp = realloc(list->data, size);
	if (tmp == NULL)
		__int_arraylist_panic("out of memory.");

	list->data = tmp;
	list->maxmemb = maxmemb;
	list->size = size;
}

static void shrink_by(struct arraylist *list, size_t by)
{
	if (by > get_unused(list))
		__int_arraylist_panic("attempted corruption.");

	resize(list, list->maxmemb - by);
}

static _Bool request_shrink(struct arraylist *list)
{
	size_t by = list->maxmemb / 2;
	if (list->maxmemb - by < MINIMUM_LIST_SIZE)
		by = list->maxmemb - MINIMUM_LIST_SIZE;

	if (get_unused(list) > by)
		return 0;

	shrink_by(list, by);
	return 1;
}

static void expand_by(struct arraylist *list, size_t by)
{
	resize(list, list->maxmemb + by);
}

static void expand(struct arraylist *list)
{
	const size_t c = compress(list);
	const size_t by = list->maxmemb;

	if (c >= by)
		return;

	expand_by(list, by - c);
}

struct arraylist arraylist_create_full(size_t datasize, size_t nmemb,
				       _Bool align, struct arraylist_map map)
{
	struct arraylist list = { 0 };

	list.idx = 0; // next element placement
	list.nmemb = 0; // the amount of actual members
	list.maxmemb = nmemb; // the amount of members allocated for
	list.dsize = datasize;
	list.esize = align ? list.dsize : calc_alignment(list.dsize);
	list.size = list.esize * list.maxmemb;
	list.data = calloc(list.maxmemb, list.esize);
	list.zero = calloc(1, list.esize);

	arraylist_provide_mapping(&list, map);

	return list;
}

struct arraylist arraylist_create_preloaded(size_t datasize, size_t nmemb,
					    _Bool align)
{
	return arraylist_create_full(datasize, nmemb, align,
				     (struct arraylist_map){ 0 });
}

struct arraylist arraylist_create(size_t datasize, _Bool align)
{
	return arraylist_create_preloaded(datasize, MINIMUM_LIST_SIZE, align);
}

void arraylist_ensure(struct arraylist *list, size_t nmemb_new)
{
	if (get_unused(list) < nmemb_new)
		expand_by(list, nmemb_new - (list->maxmemb + list->nmemb));
}

void arraylist_destroy(struct arraylist *list)
{
	if (list->data != NULL)
		free(list->data);

	if (list->zero != NULL)
		free(list->zero);

	memset(list, 0, sizeof *list);
}

static void __int_arraylist_overwrite(struct arraylist *list, size_t idx,
				      const void *entry)
{
	memcpy(list->data + idx * list->esize, entry, list->dsize);
}

void arraylist_overwrite(struct arraylist *list, size_t idx, const void *entry)
{
	__int_arraylist_overwrite(list, idx, entry);
}

size_t arraylist_pushback(struct arraylist *list, const void *entry)
{
	if (full(list))
		expand(list);

	__int_arraylist_overwrite(list, list->idx++, entry);
	list->nmemb++;

	return list->idx - 1;
}

void arraylist_remove(struct arraylist *list, size_t idx)
{
	memset(list->data + list->esize * idx, 0, list->esize);
	list->nmemb--;
}

void arraylist_trim(struct arraylist *list)
{
	compress(list);
	shrink_by(list, get_unused(list));
}

void *arraylist_get(struct arraylist *list, size_t idx)
{
	return list->data + idx * list->esize;
}

void arraylist_provide_mapping(struct arraylist *list, struct arraylist_map map)
{
	memcpy(&list->map, &map, sizeof map);
}

size_t arraylist_id_locate(struct arraylist *list, unsigned id)
{
	compress(list);

	const size_t offset = list->map.id.offset;
	const size_t length = list->map.id.length;

	for (size_t i = 0; i < list->idx; ++i) {
		const void *memb = list->data + i * list->esize;
		if (memcmp(memb + offset, &id, length) == 0)
			return i;
	}

	return -1U;
}
