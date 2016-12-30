CFLAGS += -std=c89 -Wall

prefix ?= /usr/local
bindir ?= $(prefix)/bin

.PHONY: all check clean install uninstall

all: jsinc

check: jsinc
	./jsinc 2> /dev/null
	./jsinc -v > /dev/null
	./jsinc --version > /dev/null
	./jsinc fixtures/short.txt | diff -u fixtures/short.txt.js -
	./jsinc fixtures/long.txt  | diff -u fixtures/long.txt.js  -
	./jsinc -m global   fixtures/long.txt | diff -u fixtures/long.txt.js -
	./jsinc -m amd      fixtures/long.txt \
		| diff -u   fixtures/long.txt-amd.js -
	./jsinc -m commonjs fixtures/long.txt \
		| diff -u   fixtures/long.txt-commonjs.js -
	./jsinc -m es6      fixtures/long.txt \
		| diff -u   fixtures/long.txt-es6.js -
	./jsinc -m none     fixtures/long.txt \
		| diff -u   fixtures/long.txt-none.js -
	./jsinc fixtures/long.txt \
		-p "; define('fixtures/long.txt', function() { return " \
		-s '; });' | diff -u fixtures/long.txt-amd.js -

clean:
	rm -f jsinc

install: jsinc
	mkdir -p $(bindir)
	install jsinc $(bindir)/jsinc

uninstall:
	rm -f $(bindir)/jsinc
