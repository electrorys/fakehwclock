override CFLAGS=-Wall -O2

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

all: fakehwclock

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

fakehwclock: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) fakehwclock

install:
	-@install -d $(DESTDIR)/sbin
	mv -i $(DESTDIR)/sbin/hwclock $(DESTDIR)/sbin/hwclock.orig || true
	install -m755 fakehwclock $(DESTDIR)/sbin/hwclock
