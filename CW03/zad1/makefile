CC = gcc -Wall

all:
	make build
	make dir_tree
	make test

test:
	./find test_dir -maxdepth 1
	./find test_dir -maxdepth 1 -nftw
	./find test_dir -atime +1
	./find test_dir -atime +1 -nftw


build:
	$(CC) -o find find.c

clean_all:
	rm -f find
	rm -f -r test_dir

dir_tree:
	rm -f -r test_dir
	mkdir test_dir
	mkdir ./test_dir/dir1
	mkdir ./test_dir/dir2
	mkdir ./test_dir/dir1/dir3
	mkdir ./test_dir/dir1/dir4
	mkdir ./test_dir/dir2/dir5
	mkdir ./test_dir/dir2/dir6
	mkdir ./test_dir/dir2/dir7
	mkdir ./test_dir/dir1/dir3/dir8
	mkdir ./test_dir/dir1/dir3/dir9
	mkdir ./test_dir/dir1/dir3/dir10
	mkdir ./test_dir/dir1/dir4/dir11
	mkdir ./test_dir/dir2/dir5/dir12
	mkdir ./test_dir/dir2/dir5/dir13
	mkdir ./test_dir/dir2/dir5/dir14
	mkdir ./test_dir/dir2/dir5/dir15
	mkdir ./test_dir/dir2/dir6/dir16

	touch ./test_dir/test1.txt
	touch ./test_dir/dir2/test2.txt
	touch ./test_dir/dir1/dir3/test3.txt
	touch ./test_dir/dir1/dir4/test4.txt
	touch ./test_dir/dir2/dir5/test5.txt
	touch ./test_dir/dir2/dir6/test6.txt
	touch ./test_dir/dir2/dir7/test7.txt
	touch ./test_dir/dir1/dir3/dir9/test8.txt
	touch ./test_dir/dir1/dir3/dir10/test9.txt
	touch ./test_dir/dir1/dir4/dir11/test10.txt

	ln -s ./test_dir/dir1/dir4/dir11/test10.txt ./test_dir/test10.txt



