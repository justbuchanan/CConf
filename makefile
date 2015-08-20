
all:
	mkdir -p build
	cd build; cmake ..; make

run: all
	bin/cconf-test

clean:
	rm -rf build bin

pretty:
	stylize --exclude_dirs bin build third_party
checkstyle:
	stylize --check --exclude_dirs bin build third_party
