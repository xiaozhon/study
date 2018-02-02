/***********************************************************************
 * CSCI 503 Operating System Lab 2: IPC engine
 * 
 * Use Unix socket to do inter process communication, one process is 
 * server, the other is client
 *
 * @author: Xiaoyang Zhong
 ***********************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>

// the max number of clients that the server can hold
#define MAX_CLIENTS 5
// seed is the max number fo rand function
#define SEED 512
// the total number of messages client will send
#define TOTAL_MSG 5000000

/**
 * generate a random integer
 * 
 * @arguments 
 *		int n: n acts as the seed of the generator
 * @return
 *		a random number in the range 0 ~ (n - 1)
**/
int gen_rand(int n) { //return 0 : n -1
	return  ( ((double) rand()) / RAND_MAX ) * n;
}

/**
 * get system's current time
 * 
 * @return
 *		system's current time in seconds
**/
double get_cur_time() {
	struct timeval   tv;
	struct timezone  tz;
	double cur_time;
	
	if(gettimeofday(&tv, &tz) < 0) {
		printf("Get System Time Error! \n");
		exit(1);
	}

	cur_time = tv.tv_sec + tv.tv_usec / 1000000.0;

	return cur_time;
}

/**
 * main function
**/
int main(int argc, char* argv[]) {
	int s_status, c_status;
	char* currentPath = getcwd(NULL, 0);
//	printf("current path is : %s \n", currentPath);	
	// the socket address
	currentPath = strcat(currentPath, "/tmp_socket");	
	
	if (fork() == 0) {	// server process (application process)
		int s, s_accept;	// sockets
		socklen_t addr_len;		// remote socket address length
		int done;
		int buf = 0;			// msg buffer, simply an integer
		double server_sum = 0;	// the sum of all msgs
		int rcvLen;				// the length of received data
		struct sockaddr_un local_addr;	// local socket address
		struct sockaddr remote;			// remote socket address (client address)
		
		// call socket
		if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {	// create socket
			perror("Server socket");
			exit(1);
		}
		local_addr.sun_family = AF_UNIX;
		strcpy(local_addr.sun_path, currentPath);
		unlink(local_addr.sun_path);
		
		// call bind
		if (bind(s, (struct sockaddr*) &local_addr, sizeof(local_addr)) < 0 ){
			perror("Server bind");
			exit(1);
		}
		
		// call listen
		if(listen(s, MAX_CLIENTS) < 0) {
			perror("Server listen");
			exit(1);
		}
		
		// call accept
		addr_len = sizeof(struct sockaddr_un);
		s_accept = accept(s, &remote, &addr_len);
		
	//	printf("Server: server started! \n");
		done = 0;
		printf("Server: start receiving \n");
		do {
			// receive message
			
			rcvLen = recv(s_accept, &buf, sizeof(buf), 0);
			if(rcvLen <= 0) {
				if(rcvLen < 0)
					perror("Server recv");
				done = 1;
			}

			if(!done) {
		//		printf("received data is: %d \n", buf);
				if(buf == SEED + 1) {	// correct data is in range 0 ~ (SEED - 1)
										// SEED + 1 is used as the "special" message,  
										// indicating the end of the sending
					done = 1;
				} else {
					server_sum = server_sum + buf;
				//	printf("server sum is: %1.0f \n", server_sum);
				}
			}
		} while(!done);
		
		printf("Server: the sum is: %1.0f \n", server_sum);
		
		close(s_accept);
		
	} else if (fork() == 0) {	// client process (driver process)
		sleep(1);	// sleep for a while
		int c;		// client socket
		int i;
		struct sockaddr_un server_addr;
		int buf = 0; 	// message buffer, an integer
		double client_sum = 0;
		
		if((c = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
			perror("Client socket");
			exit(1);
		}
		
		printf("Client: start to connect...\n");
		
		server_addr.sun_family = AF_UNIX;
		strcpy(server_addr.sun_path, currentPath);
		// connect to server	
		if(connect(c, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
			perror("Client connect");
			exit(1);
		}
		printf("Client: connected! \n");
		
		// to compute the throughput, record system's current time before starting
		double start_time = get_cur_time();
		
		// send message for TOTAL_MSG times
		for(i = 0; i < TOTAL_MSG; i++) {
		//	sleep(1);
			buf = gen_rand(SEED);	// generated a integer in the range 0 ~ SEED - 1
			// send message
			if(send(c, &buf, sizeof(buf), 0) < 0) {
				perror("Client send");
				exit(1);
			}
			
			client_sum = client_sum + buf;	// summation
		//	printf("client_sum is: %1.0f \n", client_sum);
		}
		// record time after finish sending
		double finish_time = get_cur_time();
//		printf("start time is:\t %f \n", start_time);
//		printf("finish time is:\t %f \n", finish_time);
		
		buf = SEED + 1;	// the number larger than SEED means it is the final message
						// that is, after this message, both process with exit
		// send "special" message
		if(send(c, &buf, sizeof(buf), 0) < 0) {
			perror("Client send");
			exit(1);
		}
		printf("Client: The sum is: %1.0f \n", client_sum);
		sleep(0.5);
		double time_used = finish_time - start_time;
		int throughput = TOTAL_MSG / time_used;
		printf("Total time: %f seconds, Throughput: %d msgs / second. \n", time_used, throughput);
		
		// close socket	
		close(c);
	} else {	// main process
		wait(&s_status);
		wait(&c_status);
		
	}
	
	return 0;
}
