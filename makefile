
all: CMakeLists.txt
	mkdir -p build
	cd build; cmake ..; make

run: all
	build/cconf-test

clean:
	rm -rf build

