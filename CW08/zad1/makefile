CC = gcc -Wall

run:
	./program 1 sign brain_398.ascii.pgm histograms/hist_sign_1
	./program 1 block brain_398.ascii.pgm histograms/hist_block_1
	./program 1 interleaved brain_398.ascii.pgm histograms/hist_interleaved_1

	./program 2 sign brain_398.ascii.pgm histograms/hist_sign_2
	./program 2 block brain_398.ascii.pgm histograms/hist_block_2
	./program 2 interleaved brain_398.ascii.pgm histograms/hist_interleaved_2

	./program 4 sign brain_398.ascii.pgm histograms/hist_sign_4
	./program 4 block brain_398.ascii.pgm histograms/hist_block_4
	./program 4 interleaved brain_398.ascii.pgm histograms/hist_interleaved_4

	./program 8 sign brain_398.ascii.pgm histograms/hist_sign_8
	./program 8 block brain_398.ascii.pgm histograms/hist_block_8
	./program 8 interleaved brain_398.ascii.pgm histograms/hist_interleaved_8

build:
	$(CC) -o program program.c -lpthread -lm


clean_all:
	rm -f program



