MAKE_DIR=$(PWD)

RDIR:=$(MAKE_DIR)
IDIR:=$(RDIR)/include
EDIR:=$(RDIR)/engine
GDIR:=$(RDIR)/game
ODIR:=obj

EODIR:=$(EDIR)/$(ODIR)
GODIR:=$(GDIR)/$(ODIR)

INCLUDES=$(wildcard $(IDIR)/*.h)
EOBJS=$(wildcard $(EODIR)/*.o)
GOBJS=$(wildcard $(GODIR)/*.o)

BINPATH:=$(RDIR)/bin
BIN:=$(BINPATH)/mathline

CC=clang
CFLAGS:=
CFLAGS+=-I$(IDIR)/
CFLAGS+=-std=gnu2x -Wall -Wextra -ggdb
CFLAGS+=-O3

#LDFLAGS+=-mwindows

LIBS:=-lraylib -lm

export RDIR IDIR ODIR CC CFLAGS INCLUDES

$(BIN): all | $(BINPATH)
	$(CC) -o $@ $(EOBJS) $(GOBJS) $(LIBS) $(LDFLAGS)

all:
	$(MAKE) -C engine -f engine.mk
	$(MAKE) -C game -f game.mk

$(BINPATH):
	mkdir -p $@

clean:
	$(MAKE) -C engine -f engine.mk clean
	$(MAKE) -C game -f game.mk clean
	$(RM) -f $(BIN)

.PHONY: all clean

