
all: build

build:
	gcc a2.c -o a2.`uname -m` -lm

test: build
	./a2.`uname -m`

clean:
	rm a2.`uname -m`

memcheck: build
	valgrind --tool=memcheck --leak-check=full ./a2.`uname -m`
