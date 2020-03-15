CC = gcc -Wall

R_NUM_1 = 1000
R_NUM_2 = 5000

build:
	$(CC) -o program program.c

clean_all:
	rm -f program

all:
	make build
	rm -f wyniki.txt
	echo "-----$(R_NUM_1) records of 1 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_1) 1
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_1) 1 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_1) 1 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_1) 1 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_1) 1 lib stop_timer
	echo "-----$(R_NUM_2) records of 1 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_2) 1
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_2) 1 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_2) 1 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_2) 1 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_2) 1 lib stop_timer
	echo "-----$(R_NUM_1) records of 4 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_1) 4
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_1) 4 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_1) 4 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_1) 4 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_1) 4 lib stop_timer
	echo "-----$(R_NUM_2) records of 4 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_2) 4
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_2) 4 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_2) 4 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_2) 4 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_2) 4 lib stop_timer
	echo "-----$(R_NUM_1) records of 512 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_1) 512
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_1) 512 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_1) 512 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_1) 512 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_1) 512 lib stop_timer
	echo "-----$(R_NUM_2) records of 512 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_2) 512
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_2) 512 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_2) 512 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_2) 512 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_2) 512 lib stop_timer
	echo "-----$(R_NUM_1) records of 1024 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_1) 1024
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_1) 1024 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_1) 1024 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_1) 1024 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_1) 1024 lib stop_timer
	echo "-----$(R_NUM_2) records of 1024 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_2) 1024
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_2) 1024 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_2) 1024 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_2) 1024 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_2) 1024 lib stop_timer
	echo "-----$(R_NUM_1) records of 4096 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_1) 4096
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_1) 4096 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_1) 4096 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_1) 4096 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_1) 4096 lib stop_timer
	echo "-----$(R_NUM_2) records of 4096 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_2) 4096
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_2) 4096 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_2) 4096 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_2) 4096 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_2) 4096 lib stop_timer
	echo "-----$(R_NUM_1) records of 8192 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_1) 8192
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_1) 8192 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_1) 8192 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_1) 8192 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_1) 8192 lib stop_timer
	echo "-----$(R_NUM_2) records of 8192 characters-----" >> "wyniki.txt"
	./program generate data $(R_NUM_2) 8192
	echo "copy using system functions" >> "wyniki.txt"
	./program start_timer copy data data_sys $(R_NUM_2) 8192 sys stop_timer
	echo "copy using library functions" >> "wyniki.txt"
	./program start_timer copy data data_lib $(R_NUM_2) 8192 lib stop_timer
	echo "sort using system functions" >> "wyniki.txt"
	./program start_timer sort data_sys $(R_NUM_2) 8192 sys stop_timer
	echo "sort using library functions" >> "wyniki.txt"
	./program start_timer sort data_lib $(R_NUM_2) 8192 lib stop_timer
