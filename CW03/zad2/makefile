CC = gcc -Wall
MIN_SIZE = 20
MAX_SIZE = 50
N = 100
P = 10
T = 1

all:
	make build
	make generate
	./macierz files/list $(P) $(T) one_file
	make test
	./macierz files/list $(P) $(T) two_files
	rm -f -r tmp

test:
	$(CC) -o test test.c
	./test files/list


build:
	$(CC) -o macierz macierz.c

generate:
	$(CC) -o generate generator.c
	mkdir -p files
	mkdir -p tmp/files
	./generate $(MIN_SIZE) $(MAX_SIZE) $(N)


clean_all:
	rm -f macierz
	rm -f generate
	rm -f test
	rm -f -r files
	rm -f -r tmp
