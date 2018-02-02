/***********************************************************************
 * CSCI 503 Operating System Lab 6: Performance Measurement
 * 
 * Context switch overhead
 * 
 * @author: Xiaoyang Zhong
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <papi.h>

/**
 * A simple main function is enough for the test.
 * Create two processes, parent and child, communicating using pipes.
 * Parent sends a number of messages to child using one pipe, upon receiving,
 * child sends back an acknowledgement using another pipe.
 *
 * The total number of messages is 1000, The number of context switches is >= 1000 * 2.
**/
int main(int argc, char* argv[]) {
	int pipe_p2c[2];	// pipe, parent send to child
	int pipe_c2p[2];	// pipe, child send to parent
	unsigned char p_msg = 'a';	// one byte message from parent to child
	unsigned char p_rcv;		// parent receive child ack
	unsigned char c_rcv;		// child receive parent msg
	unsigned char c_ack = p_msg;		// one byte ack from child to parent

	int retval;		// parent wait for child to terminate
	int N = 1000;		// the number of messages
	
	long long int start, end;	// record time
	long long int s_cycle, e_cycle;	// record cycles
	// initialize pipes
	if(pipe(pipe_p2c) < 0) {
		perror("pipe_p2c"); 
		exit(1);
	}
	if(pipe(pipe_c2p) < 0) {
		perror("pipe_p2c"); 
		exit(1);
	}
	// get time and cycle before start
	start = PAPI_get_real_usec();
	s_cycle = PAPI_get_real_cyc();

	if(fork() > 0) {
		// parent
		int i;
		int size;
		close(pipe_c2p[1]);		// close unused pipe write end
		for(i = 0; i < N; i++) {
			// write message
			write(pipe_p2c[1], &p_msg, sizeof(p_msg));
			
			// read ack from pipe_c2p
			if((size = read(pipe_c2p[0], &p_rcv, sizeof(p_rcv))) != 1) {
				perror("Parent read");
				exit(1);
			}
		}
		close(pipe_p2c[1]);
		
		// get time and cycles after the test
		end = PAPI_get_real_usec();
		e_cycle = PAPI_get_real_cyc();
		// print result
		// The result will be printed twice
		printf("pid is: %d \nStart time:\t%lld\nEnd time:\t%lld\nUsed time:\t%lld\n",
				getpid(), start, end, end - start);
		printf("pid is: %d\nStart cycle:\t%lld\nEnd cycle:\t%lld\nUsed cycles: \t%lld\n \n",
			getpid(), s_cycle, e_cycle, e_cycle - s_cycle);
		
		// wait for child to terminate
		wait(&retval);
		if (WEXITSTATUS(retval)) {
			fprintf(stderr, "Abnormal exit.\n");
		}
	} else {
		// child
		int size;
	
		close(pipe_p2c[1]);		// close unused pipe end, so the process can detect the end of the data
		
		while((size = read(pipe_p2c[0], &c_rcv, sizeof(c_rcv))) == 1) {
			// receive message from pipe_p2c
			c_ack = c_rcv;
			// write ack to pipe_c2p
			write(pipe_c2p[1], &c_ack, sizeof(c_ack));
		}
		// finished, close pipe write end
		close(pipe_c2p[1]);		
	}
	
	return 0;
}
