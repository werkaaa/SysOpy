CC = gcc -Wall

build:
	$(CC) -o catcher catcher.c
	$(CC) -o sender sender.c

clean_all:
	rm -f catcher
	rm -f sender

#example calls:
#./catcher sigqueque
#./sender PRINTED_PID 100 sigqueque
#./catcher kill
#./sender PRINTED_PID 100 kill
#./catcher sigrt
#./sender PRINTED_PID 100 sigrt

