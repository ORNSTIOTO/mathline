SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,$(ODIR)/%.o,$(SRCS))

all: $(OBJS)

$(ODIR)/%.o: %.c $(INCLUDES) | $(ODIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ODIR):
	mkdir -p $(ODIR)

clean:
	$(RM) -rf $(ODIR)

.PHONY: all clean

