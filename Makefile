CC = cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Wpedantic -Werror
PREFIX ?= /usr/local

all: gnuger

gnuger: gnuger.c
	$(CC) $(CFLAGS) -o $@ $<

test: gnuger
	./gnuger
	./gnuger list
	./gnuger explain

install: gnuger
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp gnuger $(DESTDIR)$(PREFIX)/bin/gnuger

clean:
	rm -f gnuger

.PHONY: all test install clean
