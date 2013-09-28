#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include "err.h"

#define MAX_NUMLEN 22

inline int converged(int i, int n){
	return i == n;
}

inline void clear(char* buf) {
	int i = 0;
	for(;i < MAX_NUMLEN; i++)
		buf[i] = '\0';
}

int main(int argc, char** argv) {
    int read_dsc[2];
    int write_dsc[2];
	int n; 
    /*
     * first descriptors are for Ai, second for Ai-1
     */
    scanf("%d %d %d %d %d", &write_dsc[0], &read_dsc[0], 
                          &write_dsc[1], &read_dsc[1], &n);

	/* initialiasing buffers used to communication with proccesses A */
	char buf_a[MAX_NUMLEN];
	char buf_a_prev[MAX_NUMLEN];
	clear(buf_a);
    clear(buf_a_prev);

    long int num1, num2;
	num1 = num2 = 0;
	int conv_counter = 0;
    /* 
     * after n exchanges we are certain, that the processes
     * have sorted numbers
     */
	while(!converged(conv_counter++, n)) {
		if(read(read_dsc[0], buf_a, MAX_NUMLEN) == -1)          syserr("Error in reading from Ai");
		if(read(read_dsc[1], buf_a_prev, MAX_NUMLEN) == -1)     syserr("Error in reading from Ai-1");
		sscanf(buf_a, "%ld", &num2);
		sscanf(buf_a_prev, "%ld", &num1);

        /* making sure that smaller number is in buf_a_prev */
		if(num1 > num2) {
			clear(buf_a); clear(buf_a_prev);
			sprintf(buf_a, "%ld", num1);
			sprintf(buf_a_prev, "%ld", num2);
		}
        /* passing numbers to proccesses A */
		if(write(write_dsc[0], buf_a, MAX_NUMLEN) == -1)        syserr("Error in writing to Ai");
		if(write(write_dsc[1], buf_a_prev, MAX_NUMLEN) == -1)   syserr("Error in writing to Ai-1");
        /* cleaning buffers */
		clear(buf_a);
		clear(buf_a_prev);
	}

    /* cleaning */
    if(close(read_dsc[0]) == -1)    syserr("Error in close(read_dsc[0]), B");
    if(close(read_dsc[1]) == -1)    syserr("Error in close(read_dsc[1]), B");
    if(close(write_dsc[0]) == -1)   syserr("Error in close(write_dsc[0]), B");
    if(close(write_dsc[1]) == -1)   syserr("Error in close(write_dsc[1]), B");
    return 0;
}
