SRCS=$(wildcard src/*.md.in)
# DOCS=$(SRCS:%.md.in=build/%.md)
DOCS=$(subst src,build,$(SRCS:.md.in=.md))

build/%.md : src/%.md.in
	pandoc --filter pandoc-citeproc \
	       --bibliography=bib.bib \
	       --toc \
	       -M csl=ieee.csl \
	       -M link-citations=true \
	       -s $< \
	       -t gfm \
	       -o $@

all: $(DOCS)
	mkdir -p build
	echo $(DOCS)

clean:
	rm -f $(DOCS)

