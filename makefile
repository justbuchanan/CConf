
all:
	mkdir -p build
	cd build; cmake ..; make

run: all
	bin/cconf-test

clean:
	rm -rf build
