/***********************************************************************
 * CSCI 503 Operating System Lab 6: Performance Measurement
 * 
 * System call overhead
 * 
 * @author: Xiaoyang Zhong
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "papi.h"

/**
 * A simple main function is enough for this test.
 * Call getpid() for 1000,000 times, measure the time and cpu cycles
**/
int main(int argc, char* argv[]) {
	int i;
	int pid;
	long long int start, end;	// record the time
	long long int s_cycle, e_cycle;	// record cpu cycles
	
	// get time and cpu cycle before the system call
	start = PAPI_get_real_usec();
	s_cycle = PAPI_get_real_cyc();
	
	// call getpid()
	for(i = 0; i < 1000000; i++) {
		pid = getpid();
	}
	// get time after the rest
	end = PAPI_get_real_usec();
	e_cycle = PAPI_get_real_cyc();
	
	// print results
	printf("\nStart time:\t%lld\nEnd time:\t%lld\nUsed time:\t%lld\n",
		start, end, end - start);
	printf("\nStart cycle:\t%lld\nEnd cycle:\t%lld\nUsed cycles: \t%lld\n \n",
		s_cycle, e_cycle, e_cycle - s_cycle);
	return 0;
}
