CC = gcc -Wall

run:
	gnome-terminal -- ./server 2000 socket
	gnome-terminal -- ./client LOCAL first socket
	gnome-terminal -- ./client NOTLOCAL second 127.0.0.1 2000

build:
	$(CC) -o server server.c messaging.c game.c -lpthread
	$(CC) -o client client.c messaging.c -lpthread


clean_all:
	rm -f server
	rm -f client




