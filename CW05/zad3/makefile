CC = gcc -Wall

run:
	./program test.txt

build:
	$(CC) -o consumer consumer.c
	$(CC) -o producer producer.c
	$(CC) -o program program.c

clean_all:
	rm -f consumer
	rm -f producer
	rm -f program
