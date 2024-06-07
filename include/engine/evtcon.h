#ifndef __EVTCON_H__
#define __EVTCON_H__

#include <stddef.h>
#include <stdarg.h>

typedef void (*fevtcon_t)(void *args);

struct connection {
	fevtcon_t f;
	struct event *evtgroup;

	_Bool connected;
};

struct event {
	size_t ncon;
	size_t size;
	struct connection *connections;
};

int evt_connect(struct event *evt, fevtcon_t f);

int evt_disconnect(struct connection *con);

void evt_fire(struct event *evt, void *args);

#endif
