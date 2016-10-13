# EstEID Browser Plugin

default: build

.DEFAULT:
	$(MAKE) -f Makefile.$(shell uname) $@


