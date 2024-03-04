all: run

clean:
	rm -rf build/
	mkdir build/

build: clean
	clang -mrdrnd src/test.c -o build/test
	clang -mrdrnd src/test.c -o build/test_d

debug: build
	lldb -Q -o run build/test

run: build
	build/test
