#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>
#include<fcntl.h>
#include<unistd.h>
#include <string.h>
#include "err.h"

#define MAX_NUMLEN 22


inline int converged(int i, int n) {
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
    int to_sort_dsc;
	int n;
    int numbers_desc;
	/* 
     * first desecriptors are for Bi+1, second for Bi
     */
    scanf("%d %d %d %d %d %d %d", &read_dsc[0], &write_dsc[0], 
                &read_dsc[1], &write_dsc[1], &to_sort_dsc, &n,
                &numbers_desc);	

	long int read_number;
    char one_char_buf[1];
    char num[MAX_NUMLEN];
    int i = 0;

    /* ommiting whitespaces */
    while(1 == 1) {
        read(numbers_desc, one_char_buf, 1);
        if(!isspace(one_char_buf[0]))
            break;
    }
    /* reading number from numbers_desc */
    num[i++] = one_char_buf[0];
    while (1 == 1) {
        read(numbers_desc, one_char_buf, 1);
        if(isspace(one_char_buf[0]))
            break;
        num[i++] = one_char_buf[0];
    }

    /* closing file */
    if(close(numbers_desc) == -1) syserr("Error in close(numbers_desc)");
    char* read_flag = "I\'M DONE";
    if(write(to_sort_dsc, read_flag, strlen(read_flag)) == -1)
								  syserr("Error in passing flag, B");
    num[i] = '\0';
    read_number = atol(num);
    
    /* initialiasing buffers used to communication with proccesses B */
	char buf_b[MAX_NUMLEN];
	char buf_b_prev[MAX_NUMLEN];
	clear(buf_b); 
    clear(buf_b_prev);
	
	/* sending number to Bi and Bi+1 */
	sprintf(buf_b, "%ld", read_number);
	if(write(write_dsc[0], buf_b, MAX_NUMLEN) == -1)    syserr("Error in passing nr to Bi+1");
	if(write(write_dsc[1], buf_b, MAX_NUMLEN) == -1)    syserr("Error in passing nr to Bi");
	clear(buf_b);

	long int num1, num2;
	num1 = num2 = 0;

	int conv_counter = 0;
	/* exchanging numbers with Bi and Bi+1 processes 
     *
     * after n exchanges we are certain, that the processes
     * have sorted numbers
     */	
	while(!converged(conv_counter++, n)) {
		if(read(read_dsc[0], buf_b, MAX_NUMLEN) == -1)       syserr("Error in reading from Bi+1");
		if(read(read_dsc[1], buf_b_prev, MAX_NUMLEN) == -1)  syserr("Error in reading from Bi");
		sscanf(buf_b_prev, "%ld", &num1);
		sscanf(buf_b, "%ld", &num2);
		/* making sure that smaller number is in buf_b */
		if(num1 > num2) {
			clear(buf_b);
			clear(buf_b_prev);
			sprintf(buf_b, "%ld", num2);
			sprintf(buf_b_prev, "%ld", num1);
		}
        /* passing numbers to proccesses B */
		if(conv_counter != n){
		    if(write(write_dsc[1], buf_b, MAX_NUMLEN) == -1)       syserr("Error in writing to Bi+1");
		    if(write(write_dsc[0], buf_b_prev, MAX_NUMLEN) == -1)  syserr("Error in writing to Bi");
		}
        /* cleaning buffers */
		clear(buf_b);
		clear(buf_b_prev);
	}

	char result[MAX_NUMLEN];
	clear(result);
    sprintf(result, "%ld", num1);
    /* returning result to SORT */
	write(to_sort_dsc, result, strlen(result));

    /* cleaning */
    if(close(read_dsc[0]) == -1)    syserr("Error in close(read_dsc[0]), A");
    if(close(read_dsc[1]) == -1)    syserr("Error in close(read_dsc[1]), A");
    if(close(write_dsc[0]) == -1)   syserr("Error in close(write_dsc[0]), A");
    if(close(write_dsc[1]) == -1)   syserr("Error in close(write_dsc[1]), A");
    if(close(to_sort_dsc) == -1)    syserr("Error in close(to_sort_dsc), A");
    return 0;
}
