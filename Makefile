all: sort B_border B_process A_process


sort: sort.o err.o
	cc -Wall -o sort sort.o err.o

B_border: B_border.o err.o
	cc -Wall -o B_border B_border.o err.o

B_process: B_process.o err.o
	cc -Wall -o B_process B_process.o err.o

A_process: A_process.o err.o
	cc -Wall -o A_process A_process.o err.o
    
sort.o: sort.c err.h
	cc -Wall -c sort.c

B_border.o: B_border.c
	cc -Wall -c B_border.c

B_process.o: B_process.c
	cc -Wall -c B_process.c

A_process.o: A_process.c err.h
	cc -Wall -c A_process.c


clean:
	rm -f *.o sort B_border B_process A_process

