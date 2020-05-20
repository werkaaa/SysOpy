CC = gcc -Wall

run:
	rm -f socket
	gnome-terminal -- ./server 2000 socket
	gnome-terminal -- ./client LOCAL first_client socket
	gnome-terminal -- ./client NOTLOCAL second_client 127.0.0.1 2000

build:
	$(CC) -o server server.c messaging.c game.c -lpthread
	$(CC) -o client client.c messaging.c -lpthread


clean_all:
	rm -f server
	rm -f client
	rm -f socket



