#ifndef __ARRAYLIST_H__
#define __ARRAYLIST_H__

#include <stddef.h>

#define __packed __attribute__((__packed__))

struct __packed arraylist {
	//   THOSE MEMBERS SHOULD NOT BE ACCESSED IN ANY WAY outside of the
	// implementation source. They are strictly internal and not intended
	// to be used elsewhere, not even read. Use the functions below instead.
	void *data;
	void *zero;
	size_t size;
	size_t nmemb;
	size_t maxmemb;
	size_t idx;
	size_t dsize;
	size_t esize;

	struct __packed arraylist_map {
		struct __packed {
			size_t offset;
			size_t length;
		} id;
	} map;

	//   Those flags are the only members you may modify. They control
	// the behavior of the array list.
	struct __packed arraylist_flags {
		// TODO UNFINISHED
		_Bool predictable_indexes;
	} flags;
};

struct arraylist arraylist_create_full(size_t datasize, size_t nmemb,
				       _Bool align, struct arraylist_map map);
struct arraylist arraylist_create_preloaded(size_t datasize, size_t nmemb,
					    _Bool align);
struct arraylist arraylist_create(size_t datasize, _Bool align);
void arraylist_destroy(struct arraylist *list);

void arraylist_provide_mapping(struct arraylist *list,
			       struct arraylist_map map);

void arraylist_clear(struct arraylist *list);
void arraylist_ensure(struct arraylist *list, size_t nmemb_new);
size_t arraylist_pushback(struct arraylist *list, const void *entry);
void arraylist_remove(struct arraylist *list, size_t idx);
void arraylist_overwrite(struct arraylist *list, size_t idx, const void *entry);
void arraylist_trim(struct arraylist *list);

void *arraylist_get(struct arraylist *list, size_t idx);
size_t arraylist_count(struct arraylist *list);

size_t arraylist_id_locate(struct arraylist *list, unsigned id);

#endif
