CC = gcc -Wall
MIN_SIZE = 30
MAX_SIZE = 50
N = 100
P = 10
T = 5

all:
	make build
	make -B generator
	./macierz files/list $(P) $(T) one_file
	make -B test
	rm -f files/out*
	./macierz files/list $(P) $(T) more_files
	make -B test
	rm -f -r tmp
	rm -f files/list

test:
	./test files/list


build:
	$(CC) -o macierz macierz.c
	$(CC) -o test test.c
	$(CC) -o generator generator.c

generator:
	mkdir -p files
	mkdir -p tmp/files
	./generator $(MIN_SIZE) $(MAX_SIZE) $(N)


clean_all:
	rm -f macierz
	rm -f generator
	rm -f test
	rm -f -r files
	rm -f -r tmp
