SRCS=$(wildcard src/*.md.in)
DOCS=$(subst src,dist,$(SRCS:.md.in=.md))

dist/%.md : src/%.md.in
	pandoc --filter pandoc-citeproc \
	       --bibliography=bib.bib \
	       --toc \
	       -M csl=ieee.csl \
	       -M link-citations=true \
	       -s $< \
	       -f markdown_strict+simple_tables+table_captions \
	       -t gfm \
	       -o $@

dist: dir $(DOCS)

dir:
	mkdir -p dist

.PHONY: clean
clean:
	rm -f $(DOCS)

