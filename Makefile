#ifeq ($(shell which clang >/dev/null 2>&1; echo $$?), 0)
#	CC=clang
#else
#	CC=gcc
#endif

MAKE_DIR=$(PWD)

RDIR:=$(MAKE_DIR)
IDIR:=$(RDIR)/include
GDIR:=$(RDIR)/game
WDIR:=$(RDIR)/window
ODIR:=obj

WODIR:=$(WDIR)/$(ODIR)
GODIR:=$(GDIR)/$(ODIR)

INCLUDES=$(wildcard $(IDIR)/*.h)
WOBJS=$(wildcard $(WODIR)/*.o)
GOBJS=$(wildcard $(GODIR)/*.o)

BIN:=$(RDIR)/bin/ballgame

CC=clang
CFLAGS:=
CFLAGS+=-I$(IDIR)/
CFLAGS+=-std=gnu2x -Wall -Wextra -ggdb
CFLAGS+=-O3

LIBS:=-lraylib -lm

export RDIR IDIR ODIR CC CFLAGS INCLUDES

$(BIN): all
	$(CC) $(LIBS) -o $@ $(WOBJS) $(GOBJS)

all:
	$(MAKE) -C window -f window.mk
	$(MAKE) -C game -f game.mk

clean:
	$(MAKE) -C window -f window.mk clean
	$(MAKE) -C game -f game.mk clean
	$(RM) -f $(BIN)

.PHONY: all clean

