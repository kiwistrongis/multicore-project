# globals
default: paper
freshen: clean paper
clean:
	rm -rf gen/*
	rm -rf *.pdf

# vars
viewer = evince

# lists
examples_c = \
	gen/thread.c \
	gen/ticket-simulator.c \
	gen/stock-exchange.c
examples_rs = \
	gen/thread.rs \
	gen/ticket-simulator.rs \
	gen/stock-exchange.rs
paper = gen/rust-concurrency.pdf
package_file = rust-concurrency.tgz

# compilation definitions
$(examples_c): gen/%: examples/src/%
	sed 's/\t/  /g' $< > $@
$(examples_rs): gen/%: examples/src/bin/%
	sed 's/\t/  /g' $< > $@

$(paper): gen/%.pdf: src/%.md \
		$(examples_c) $(examples_rs) \
		src/meta.yaml \
		src/template.tex
	pandoc \
		--latex-engine=xelatex \
		--latex-engine-opt='-shell-escape' \
		--template src/template.tex \
		--filter pandoc-include-code \
		--highlight-style=tango \
		-r markdown \
		src/meta.yaml \
		$< -o $@

$(package_file): $(paper)
	cp $(paper) \
		"Synchronization and Concurrency in the Rust Programming Language.pdf"
	tar -cvzf $(package_file) \
		examples gen notes src \
		makefile license.md *.pdf
pkg: $(package_file)

# commands
all: paper
paper: $(paper)

test: $(paper)
	$(viewer) $<

ci:
	make-ci all $$(find src examples/src/bin)
