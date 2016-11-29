CFLAGS += -std=c89 -Wall

prefix ?= /usr/local
bindir ?= $(prefix)/bin

.PHONY: all check clean install uninstall

all: jsinc

check: jsinc
	./jsinc -v > /dev/null
	./jsinc --version > /dev/null
	./jsinc sample.txt | diff -u sample.txt.js -
	./jsinc sample2.txt \
		-p 'define("sample2.txt", function() { return' \
		-P '; });' | diff -u sample2.txt.js -

clean:
	rm -f jsinc

install: jsinc
	mkdir -p $(bindir)
	install jsinc $(bindir)/jsinc

uninstall:
	rm -f $(bindir)/jsinc
