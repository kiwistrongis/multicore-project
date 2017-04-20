# globals
default: build
all: build build-opt
build: build-rs build-c
build-opt: build-opt-rs build-opt-c
freshen: clean build
clean:
	rm -rf bin/*
	rm -rf data/{time,mem}/*
	rm -f *.tgz
	cargo clean

# variables
package_file = examples.tgz

# lists
binaries = \
	bin/thread \
	bin/ticket-simulator \
	bin/stock-exchange
binaries_opt = \
	bin/thread-opt \
	bin/ticket-simulator-opt \
	bin/stock-exchange-opt

# compilation definitions
$(binaries): bin/% : src/%.c
	gcc -lm -fopenmp \
		$< -o $@
$(binaries_opt): bin/%-opt : src/%.c
	gcc -lm -fopenmp -O3 \
		$< -o $@

# basic commands
build-rs:
	cargo build
build-c: $(binaries)
build-opt-rs:
	cargo build --release
build-opt-c: $(binaries_opt)

$(package_file): $(binaries)
	tar -cvzf $(package_file) \
		bin data src \
		makefile readme.md
pkg: $(package_file)

ci:
	make-ci build  $$(find src)

record: build-opt
	src/timing.sh
	src/valgrind.sh
