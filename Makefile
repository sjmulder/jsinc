CFLAGS += -std=c89 -Wall

DISTNAME ?= jsinc

prefix ?= /usr/local
bindir ?= $(prefix)/bin

.PHONY: all check clean install uninstall

all: jsinc

check: jsinc
	./jsinc 2> /dev/null
	./jsinc -v > /dev/null
	./jsinc --version > /dev/null
	./jsinc -c fixtures/short.txt | diff -u fixtures/short.txt.js -
	./jsinc -c fixtures/long.txt  | diff -u fixtures/long.txt.js  -
	./jsinc -c -m global   fixtures/long.txt \
		| diff -u      fixtures/long.txt.js -
	./jsinc -c -m amd      fixtures/long.txt \
		| diff -u      fixtures/long.txt-amd.js -
	./jsinc -c -m commonjs fixtures/long.txt \
		| diff -u      fixtures/long.txt-commonjs.js -
	./jsinc -c -m es6      fixtures/long.txt \
		| diff -u      fixtures/long.txt-es6.js -
	./jsinc -c -m none     fixtures/long.txt \
		| diff -u      fixtures/long.txt-none.js -
	./jsinc -c fixtures/long.txt \
		-p "; define('fixtures/long.txt', function() { return " \
		-s '; });' | diff -u fixtures/long.txt-amd.js -

clean:
	rm -f jsinc $(DISTNAME)

dist: $(DISTNAME).zip

$(DISTNAME).zip: jsinc
	rm -f $(DISTNAME).zip
	zip $(DISTNAME).zip Readme.md Changelog.md jsinc

install: jsinc
	mkdir -p $(bindir)
	install jsinc $(bindir)/jsinc

uninstall:
	rm -f $(bindir)/jsinc
