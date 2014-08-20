
all: CMakeLists.txt
	mkdir build
	cd build; cmake ..; make

run: all
	build/cconf-test

clean:
	rm -rf build

