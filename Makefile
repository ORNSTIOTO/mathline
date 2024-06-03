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

BINPATH:=$(RDIR)/bin
BIN:=$(BINPATH)/mathline

CC=clang
CFLAGS:=
CFLAGS+=-I$(IDIR)/
CFLAGS+=-std=gnu2x -Wall -Wextra -ggdb
CFLAGS+=-O3

LIBS:=-lraylib -lm

export RDIR IDIR ODIR CC CFLAGS INCLUDES

$(BIN): all | $(BINPATH)
	$(CC) -o $@ $(WOBJS) $(GOBJS) $(LIBS)

all:
	$(MAKE) -C window -f window.mk
	$(MAKE) -C game -f game.mk

$(BINPATH):
	mkdir -p $@

clean:
	$(MAKE) -C window -f window.mk clean
	$(MAKE) -C game -f game.mk clean
	$(RM) -f $(BIN)

.PHONY: all clean

