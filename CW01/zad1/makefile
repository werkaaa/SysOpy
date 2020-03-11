CC = gcc
LIB_NAME = ldiff
SOURCE_NAME = diff


static:
	$(CC) -Wall -c $(SOURCE_NAME).c -o $(LIB_NAME).o
	ar rcs $(LIB_NAME).a $(LIB_NAME).o
	make clean

shared:
	$(CC) -Wall -fPIC -c $(SOURCE_NAME).c -o $(LIB_NAME).o
	$(CC) -fPIC -shared -o $(LIB_NAME).so $(LIB_NAME).o
	make clean

clean:
	rm -f *.o

clean_all:
	rm -f *.o *.a *.so