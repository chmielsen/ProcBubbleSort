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
    int read_dsc;
    int write_dsc;
	int n;
    /* reading descriptors to communicate with one process A */
    scanf("%d %d %d", &write_dsc, &read_dsc, &n); 
	char buf[MAX_NUMLEN];
	int conv_counter = 0;

    /* 
     * after n exchanges we have are certain, that the processes
     * have sorted numbers
     */
	while(!converged(conv_counter++, n)) {
		clear(buf);
        int len;
		if((len = read(read_dsc, buf, MAX_NUMLEN)) == -1)   syserr("Error in reading, B_border");
		if(write(write_dsc, buf, len) == -1)                syserr("Error in writing, B_border");
	}

    /* cleaning */
    close(read_dsc);
    close(write_dsc);
    return 0;
}
