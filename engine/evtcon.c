#include "engine/evtcon.h"
#include <malloc.h>
#include <string.h>

#define MAX(x, y) x > y ? x : y

#define MINIMUM_CONNECTION_SIZE 4

static int evt_resize_to(struct event *evt, size_t n)
{
	const size_t to = MAX(n, MINIMUM_CONNECTION_SIZE);
	void *tmp = realloc(evt->connections, to * sizeof *evt->connections);
	if (tmp == NULL)
		return -1;

	evt->connections = tmp;
	return 0;
}

static int evt_table_maintenance(struct event *evt, _Bool is_inserting)
{
	if (is_inserting && evt->ncon + 1 >= evt->size)
		return evt_resize_to(evt, evt->size * 2);

	if (!is_inserting && evt->ncon <= evt->size / 2 &&
	    evt->ncon >= MINIMUM_CONNECTION_SIZE)
		return evt_resize_to(evt, evt->size / 2);

	return 0;
}

int evt_connect(struct event *evt, fevtcon_t f)
{
	if (evt_table_maintenance(evt, 1) < 0)
		return -1;

	struct connection *con = &evt->connections[evt->ncon++];
	con->f = f;
	con->evtgroup = evt;
	con->connected = 1;

	return 0;
}

int evt_disconnect(struct connection *con)
{
	if (evt_table_maintenance(con->evtgroup, 0) < 0)
		return -1;

	con->connected = 0;

	struct event *evt = con->evtgroup;
	const size_t idx = (size_t)(con - evt->connections) / sizeof *con;
	const size_t cutoff = sizeof *con * (--evt->ncon - idx);
	memmove(&con[0], &con[1], cutoff);

	return 0;
}

void evt_fire(struct event *evt, void *args)
{
	for (size_t i = 0; i < evt->ncon; ++i) {
		const struct connection con = evt->connections[i];
		if (con.f != NULL && con.connected)
			con.f(args);
	}
}
