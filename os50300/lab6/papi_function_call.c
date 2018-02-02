/***********************************************************************
 * CSCI 503 Operating System Lab 6: Performance Measurement
 * 
 * Function call overhead
 * 
 * @author: Xiaoyang Zhong
 ***********************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "papi.h"

/**
 * A dummy function, used to measure the overhead. the function does nothing
**/
void dummy() {
}

/**
 * Call the dummy function for 1000,000 times, measure the time and cpu cycles
**/
int main(int argc, char* argv[]) {
	int i;
	long long int start, end;	// record time
	long long int s_cycle, e_cycle;	// record cpu cycles
	
	// get time before calling the function
	start = PAPI_get_real_usec();
	s_cycle = PAPI_get_real_cyc();
	
	// call the dummy function for 1000,000 times
	for(i = 0; i < 1000000; i++) {
		dummy();
	}
	
	// get time after the test
	end = PAPI_get_real_usec();
	e_cycle = PAPI_get_real_cyc();

	// print result
	printf("\nStart time:\t%lld\nEnd time:\t%lld\nUsed time:\t%lld\n",
		start, end, end - start);
	printf("\nStart cycle:\t%lld\nEnd cycle:\t%lld\nUsed cycles: \t%lld\n \n",
		s_cycle, e_cycle, e_cycle - s_cycle);
	
	return 1;
}
