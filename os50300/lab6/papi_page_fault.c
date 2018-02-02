/***********************************************************************
 * CSCI 503 Operating System Lab 6: Performance Measurement
 * 
 * Page fault overhead
 * 
 * @author: Xiaoyang Zhong
 ***********************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "papi.h"

/**
 * The size of int is 4 bytes in all the machines we have.
 *
 * Memory size: (use 1K = 1000, the result should be very similar for 1K = 1024)
 *		cyclone: 16329416 Kbytes	-- len = 4082354000L
 *		rrpc01:	 1919784 Kbytes		-- len = 479946000L
 *
 * Define a 2-D array, the total size is the main memory size. If the main
 * memory is N MB, then the array has N subarrays, the size of each subarray is 
 * 1 MB.
**/

#define BYTE_SIZE 1919784000	// for sl253-rrpc01, array size, 1919 MB
								// this value works	

//#define BYTE_SIZE 16329416000	// for cyclone, array size, 16329 MB

#define INT_LEN 250000	// the length of array with 1 MB bytes memory
						// since each int has 4 bytes, the length of the array is
						// 250,000

/**
 * A simple main function is enough for this test
 * Define two arrays, each array is of size "Main Memory size"
**/
int main(int argc, char* argv[]) {	
	int i;
	int j;
	long long int start, end;	// record time
	long long int s_cycle, e_cycle;	// record cycles
	
	int num_mb = BYTE_SIZE / 1000000;	// The number of MBs, it is the number
										// of subarrays in the big array
	
	int **a1;	// array 1
	int **a2;	// array 2
	
	// allocate memory for the pointer to subarrays
	// the number of subarrays is num_mb
	a1 = (int**) malloc(num_mb*sizeof(int*));	
	// allocate memory for subarray
	// the size of subarray is 1000,000 = 1 MB. 
	for(i = 0; i < num_mb; i++){
		a1[i] = (int*) malloc(INT_LEN*sizeof(int));
	}
	// initialization
	for(i = 0; i < num_mb; i++){
		for(j = 0; j < INT_LEN; j++)
			a1[i][j] = random();
	}
	
	// allocate memory for array 2 using the same way
	a2 = (int**) malloc(num_mb*sizeof(int*));
	for(i = 0; i < num_mb; i++){
		a2[i] = (int*) malloc(INT_LEN*sizeof(int));
	}
	for(i = 0; i < num_mb; i++){
		for(j = 0; j < INT_LEN; j++)
			a2[i][j] = random();
	}
	int m, n, k;	// store array values
	// get time and cycle before the test
	
	start = PAPI_get_real_usec();
	s_cycle = PAPI_get_real_cyc();
	for(i = 0; i < INT_LEN; i = i + 1500){	
		// read array items, each of them is 1000,000 bytes away
		// For each array, 250,000 / 1500 = 167 elements are read.
		// total number of readings is 501
		m = a1[1][i];
		n = a1[2][i];
		k = a1[3][i];
	}
	
	// get time and cycles after the test
	end = PAPI_get_real_usec();
	e_cycle = PAPI_get_real_cyc();
	// print result
	printf("pid is: %d \nStart time:\t%lld\nEnd time:\t%lld\nUsed time:\t%lld\n",
		getpid(), start, end, end - start);
	printf("pid is: %d\nStart cycle:\t%lld\nEnd cycle:\t%lld\nUsed cycles: \t%lld\n \n",
		getpid(), s_cycle, e_cycle, e_cycle - s_cycle);
	
	// free all memories
	for(i = 0; i < num_mb; i++){
		free(a1[i]);
	}
	free(a1);
	for(i = 0; i < num_mb; i++){
		free(a2[i]);
	}
	free(a2);
	return 0;
}