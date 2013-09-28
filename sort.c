#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
//#include<sys/types.h>
#include<sys/wait.h>
#include "err.h"

#define MAX_LINE  300
#define MAX_NUM   22

/*
 * Wojciech Chmiel
 * Program simluating BubbleSort algorhitm on processes.
 * Processes A are communicatng with processes B and vice versa.
 */


/*
 * Wojciech Chmiel 305187
 * Program Sortowanie
 * Na calosc skladaja sie 4 pliki:
 *  - sort.c - wlasciwy program tworzacy inny procesy i zbierajacy wyniki
 *  - A_process.c - program wykonujacy dzialanie opisanego procesu A
 *  - B_border.c  - program wykonujacy dzialanie opisanych procesow B0 i Bn
 *  - B_process.c - program wykonujacy dzialanie pozostalych procesow B
 *
 * Calosc symuluje na swoj sposob dzialanie algorytmy BubbleSort na procesach
 */


/* closing array of descriptors leaving one open */
void close_arr(int** desc_array, int n, int which_open) {
    int i;
    for(i = 0; i < n; i++) {
        if(close(desc_array[i][0]) == -1) syserr("Error in close_arr");
        if(i != which_open)
            if(close(desc_array[i][1]) == -1) syserr("Error in close_arr");
    }
}

/* copying descriptor id */
void copy_dsc(int* from, int* to) {
    to[0] = from[0];
    to[1] = from[1];
}

/* checking if process B was created */
inline int create_b(int i) {
    return (i % 2) == 0;
}

/* clearing buffer */
void clear(char* buf, int len) {
    int i = 0;
    for(; i < len; i++)
        buf[i] = '\0';
}

int main(int argc, char** argv) {
    if(argc != 3)  syserr("Wrong number of arguments");
    char* numbers = argv[2];
    const int n = atoi(argv[1]);
    if(n < 0)      syserr("Negative number of numbers, are you serious?");

	/* checking if there is enough numbers to read */
	FILE* numbers_test = fopen(numbers, "r");
	int num_count = 0;
	long int dummy;
    int error_finder;
	while(((error_finder = fscanf(numbers_test,"%ld", &dummy)) != EOF) && (error_finder != 0)) {
        if(dummy == 0)      syserr("Wrong input, 0 found");
        num_count++;
    }
    if(error_finder == 0)   syserr("Wrong input file, bad characters");
	if(n > num_count)       syserr("Wrong first argument - not enough numbers in file to read");
	fclose(numbers_test);
    /*
     * arrays of decriptors using in communication between
     * process SORT and processes A
     */
    int** a_to_sort;
    a_to_sort = malloc(n * sizeof(int*));
    int i;
    for(i = 0; i < n; i++)
        a_to_sort[i] = malloc(2 * sizeof(int));
    for(i = 0; i < n; i++) 
       if (pipe(a_to_sort[i]) == -1) syserr("Error in a_to_sort pipe\n");  
    
    /* descriptors */
    int sort_to_proc[2];
    int a_to_b[2], a_to_b_prev[2];
    int b_to_a[2], b_to_a_prev[2];

    if(pipe(sort_to_proc) == -1) syserr("Error in B0 - pipe from sort");
    if(pipe(b_to_a) == -1)       syserr("Error in B0 - pipe to A");
    if(pipe(a_to_b) == -1)       syserr("Error in B0 - pipe to B");
    

    
    char buf[MAX_LINE]; /* variable used to pass descriptors to children */

    /* creating process B0 */
    clear(buf, MAX_LINE);
    switch(fork()) {
        case -1:
            syserr("Error in B0 - fork");
        case 0:
            /* switching stdin  */
            if(close(0) == -1)                 syserr("Error in switching stdin");
            if(dup(sort_to_proc[0]) != 0)      syserr("Error in child dup");
            if(close(sort_to_proc[0]) == -1)   syserr("Error in close(sort_to_proc[0])");
            
            /* closing all descrpitors used to communition between processes A and SORT */
            close_arr(a_to_sort, n, -1);

            /* freeing allocated memory */
            for(i = 0; i < n; i++)
                free(a_to_sort[i]);
            free(a_to_sort);

            if(close(sort_to_proc[1]) == -1)   syserr("Error in close(sort_to_proc[1])");

            /* closing redundant descriptors */
            if(close(b_to_a[0]) == -1)         syserr("Error in close(b_to_a[0])"); 
            if(close(a_to_b[1]) == -1)         syserr("Error in close(a_to_b[1])");

            execl("./B_border", "B_border", "first", (char*)0);
            syserr("Error in B0 - exec");
        default:
            if(close(sort_to_proc[0]) == -1)   syserr("Error in close(sort_to_proc[0])");
            /* passing descriptors */
            sprintf(buf, "%d %d %d\n", b_to_a[1], a_to_b[0], n);
            if(write(sort_to_proc[1], buf, MAX_LINE) == -1)
                                               syserr("Error in write, B0");
            if(close(sort_to_proc[1]) == -1)   syserr("Error in close(sort_to_proc[1])");
    }



    /* numbers - file with numbers in it */
    int numbers_desc = open(numbers, O_RDONLY);

    /* 
     * creating A1, B2 , ..., An processes
     * idea is simple, we keep opened 8 descriptors, which are necessary
     * to communicate in neighbour processes (Bi-1, Bi or Ai-1, Ai)
     */
    for(i = 1; i < 2*n; i++) {
        clear(buf, MAX_LINE);
        copy_dsc(a_to_b, a_to_b_prev);
        copy_dsc(b_to_a, b_to_a_prev);
        a_to_b[0] = a_to_b[1] = b_to_a[0] = b_to_a[1] = -1;

        if(pipe(sort_to_proc) == -1)        syserr("Error in pipe(sort_to_proc)");
        if(pipe(a_to_b) == -1)              syserr("Error in pipe(a_to_b)");
        if(pipe(b_to_a) == -1)              syserr("Error in pipe(b_to_a)");

        char dummy[20];
        /* number of process A */
        int a_proc_num = (i - 1) / 2;
        switch(fork()) {
            case -1:
                syserr("Error in fork\n");
            /* child */
            case 0:
                /* switching stdin */
                if(close(0) == -1)                syserr("Error in switching stdin");
                if(dup(sort_to_proc[0]) != 0)     syserr("Error in child dup");
                if(close(sort_to_proc[0]) == -1)  syserr("Error in close(sort_to_proc[0])");

                /* closing redundant descriptors */
                if(close(sort_to_proc[1]) == -1)  syserr("Error in close(sort_to_proc[1])");
            
                /* creating B process */
                if(create_b(i)) {
                    /* closing redundant descriptors */
                    close(numbers_desc);
                    if(close(b_to_a[0]) == -1)          syserr("Error in close(b_to_a[0]), B"); 
                    if(close(a_to_b[1]) == -1)          syserr("Error in close(a_to_b[1]), B");
                    if(close(b_to_a_prev[0]) == -1)     syserr("Error in close(b_to_a_prev[0]), B");
                    if(close(a_to_b_prev[1]) == -1)     syserr("Error in close(a_to_b_prev[1]), B");
                    close_arr(a_to_sort, n, -1);

                    /* freeing allocated memory */
                    for(i = 0; i < n; i++)
                        free(a_to_sort[i]);
                    free(a_to_sort);

                    execl("./B_process", "B_process", (char*)0);
                    syserr("Error in B - execl");
                } 
                /* creating A process */
                else {
                    /* closing redundant descriptors */
                    if(close(b_to_a[1]) == -1)          syserr("Error in close(b_to_a[1]), A"); 
                    if(close(a_to_b[0]) == -1)          syserr("Error in close(a_to_b[0]), A");
                    if(close(b_to_a_prev[1]) == -1)     syserr("Error in close(b_to_a_prev[1]), A");
                    if(close(a_to_b_prev[0]) == -1)     syserr("Error in close(a_to_b_prev[0]), A");
                    close_arr(a_to_sort, n, a_proc_num);

                    /* freeing allocated memory */
                    for(i = 0; i < n; i++)
                        free(a_to_sort[i]);
                    free(a_to_sort);

                    execl("./A_process", "A_process", (char*)0);
                    syserr("Error in A - execl");
                }
            /* parent */
            default:
                if(close(sort_to_proc[0]) == -1)  syserr("Error in close(sort_to_proc[0], SORT");
                /* order in passing: descrptors to later processes are first */
                if(create_b(i)) {
                    /* passing descriptors to B */
                    sprintf(buf, "%d %d %d %d %d\n", b_to_a[1], a_to_b[0], 
                                       b_to_a_prev[1], a_to_b_prev[0], n);
                }
                else {  /* passing descriptors to A */
                    sprintf(buf, "%d %d %d %d %d %d %d\n", b_to_a[0], a_to_b[1], 
                                                    b_to_a_prev[0], a_to_b_prev[1], 
                                                    a_to_sort[a_proc_num][1], n, numbers_desc);
                }

                if(write(sort_to_proc[1], buf, MAX_LINE) == -1)
                                                  syserr("Error in passing descriptors");
                if(close(sort_to_proc[1]) == -1)  syserr("Error in close(sort_to_proc[1])");
                if(close(a_to_b_prev[0]) == -1)   syserr("Error in close(a_to_b_prev[0])"); 
                if(close(a_to_b_prev[1]) == -1)   syserr("Error in close(a_to_b_prev[1])");
                if(close(b_to_a_prev[0]) == -1)   syserr("Error in close(b_to_a_prev[0])");
                if(close(b_to_a_prev[1]) == -1)   syserr("Error in close(b_to_a_prev[1])"); 

                /* reading flague, so we know that process A finished reading from numbers */
                if(!create_b(i)) 
                    read(a_to_sort[a_proc_num][0], dummy, 20);
        }
    }  /* process An created */

    if(close(numbers_desc) == -1)    syserr("Error in close(numbers_desc)");
    if(close(b_to_a[0]) == -1)       syserr("Error in close(b_to_a[0])");
    if(close(a_to_b[1]) == -1)       syserr("Error in close(a_to_b[1])");

    /* creating Bn */
    pipe(sort_to_proc);
    clear(buf, MAX_LINE);
    switch(fork()) {
        case -1:
            syserr("Error in Bn - fork");
        case 0:

            if(close(0) == -1)                 syserr("Error in switching stdin");
            if(dup(sort_to_proc[0]) != 0)      syserr("Error in child dup");
            if(close(sort_to_proc[0]) == -1)   syserr("Error in close(sort_to_proc[0])");

            /* closing all descrpitors used to communition between processes A and SORT */
            close_arr(a_to_sort, n, -1);

            /* freeing allocated memory */
            for(i = 0; i < n; i++)
            free(a_to_sort[i]);
            free(a_to_sort);

            if(close(sort_to_proc[1]) == -1)   syserr("Error in close(sort_to_proc[1])");

            execl("./B_border", "B_border", "last", (char*)0);
            syserr("Error in Bn - exec"); 

        default:

            if(close(sort_to_proc[0]) == -1)   syserr("Error in close(sort_to_proc[0])");
            /* passing descriptors */
            sprintf(buf, "%d %d %d\n", b_to_a[1], a_to_b[0], n);
            if(write(sort_to_proc[1], buf, MAX_LINE) == -1)
                                               syserr("Error in write, Bn");
            if(close(sort_to_proc[1]) == -1)   syserr("Error in close(sort_to_proc[1])");

    }

    /* closing redundant descriptors */
    for(i = 0; i < n; i++)
        if(close(a_to_sort[i][1]) == -1)     syserr("Error in close(a_to_sort[X][1])");
    if(close(b_to_a[1]) == -1)               syserr("Error in close(b_to_a[1])");
    if(close(a_to_b[0]) == -1)               syserr("Error in close(a_to_b[0])");

    char passed_num[MAX_NUM];
    for(i = 0; i < n; i++) {
        clear(passed_num, MAX_NUM);
        int len;
        if((len = read(a_to_sort[i][0], passed_num, MAX_NUM)) == -1)
                                             syserr("Error in reading final results");
        passed_num[len] = '\0';
        printf("%ld ", atol(passed_num));
    }
    putchar('\n');

    /* freeing allocated memory */
    for(i = 0; i < n; i++)
        free(a_to_sort[i]);
    free(a_to_sort);

    /* waiting for children to end */
    for(i = 0; i <= 2*n; i++)
        if(wait(0) == -1)  syserr("Error in wait");
    return 0;
}
