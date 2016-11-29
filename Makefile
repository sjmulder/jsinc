CFLAGS += -std=c89 -Wall

prefix ?= /usr/local
bindir ?= $(prefix)/bin

.PHONY: all check clean install uninstall

all: jsinc

check: jsinc
	./jsinc -v > /dev/null
	./jsinc --version > /dev/null
	./jsinc sample.txt | diff -u sample.txt.js -
	./jsinc sample.txt \
		-p 'define("sample.txt", function() { return' \
		-P '; });' | diff -u sample.txt-v2.js -

clean:
	rm -f jsinc

install: jsinc
	mkdir -p $(bindir)
	install jsinc $(bindir)/jsinc

uninstall:
	rm -f $(bindir)/jsinc
