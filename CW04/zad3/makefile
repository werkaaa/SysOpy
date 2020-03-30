CC = gcc -Wall

all:
	make build
	make test

test:
	./program get_child_info
	./program force_segfault
	./program send_value 17

build:
	$(CC) -o program program.c

clean_all:
	rm -f program
