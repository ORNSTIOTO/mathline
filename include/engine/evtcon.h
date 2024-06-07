#ifndef __EVTCON_H__
#define __EVTCON_H__

#include <stddef.h>

#define CONNECTION_INCREMENT 8

typedef void *(*fevtcon_t)(void *);

struct connection {
	fevtcon_t f;
};

struct event {
	size_t ncon;
	struct connection *connections;
};

int evt_connect(struct event *evt, fevtcon_t f);

int evt_disconnect(struct connection *con);

// TODO

#endif

