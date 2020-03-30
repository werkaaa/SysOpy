CC = gcc -Wall

all:
	make build
	make test

test:
	./program ignore fork
	./program handler fork
	./program mask fork
	./program pending fork
	./program ignore exec
	./program mask exec
	./program pending exec

build:
	$(CC) -o process_function process_function.c
	$(CC) -o program program.c

clean_all:
	rm -f process_function
	rm -f program

