
all: build

build:
	gcc a2.c -o a2.`uname -m` -lm

test:
	./a2.`uname -m`

clean:
	rm a2.`uname -m`

