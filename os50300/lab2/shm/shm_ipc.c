/***********************************************************************
 * CSCI 503 Operating System Lab 2: IPC engine
 * 
 * Use shared memory to do inter process communication 
 *
 * @author: Xiaoyang Zhong
 ***********************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/shm.h>

#define BUFFER_SIZE 2048
#define SEED 512
#define TOTAL_MSG 5000000

// The shared area acts as a ring buffer
struct SharedMemory {
	int head;
	int tail;
	int buf[BUFFER_SIZE];
};

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
 * create shared memory area
 * 
 * @arguments
 *		size_t shm_size: shared memory size
 *		int* shm_id: shared memory id
 * 		int is_parent: whether the caller of this function is the parent
 * @return
 *		struct SharedMemory*: the pointer to the shared memory
**/
struct SharedMemory* create_shared_memory(size_t shm_size, int* shm_id, int is_parent) {
	key_t key = 10000;	// key of the shared memory associate with shm_id
	int shm_flg = 0;
  	void *shm;		// pointer to the address of the shared memory
  	struct SharedMemory *common_area;

	// set access permissions for the common area
	// parent creates the shared area, and set the permissions
	if (is_parent) {
		shm_flg = IPC_CREAT | 0666;
	}
	
	// create shared memory, if creation fails, exit
	if ((*shm_id = shmget(key, sizeof(struct SharedMemory), shm_flg)) < 0) {
		perror("shmget error");
		exit(1);
	}

	// attach, if attaching fails, exit
	if ((shm = shmat(*shm_id, NULL, 0)) == (void *) -1) {
		perror("shmat error");
		exit(1);
	}

	common_area = (struct SharedMemory*) shm; 

	return common_area;
}

/**
 * Main function, two processes are created, and communicate using shared memory
**/
int main(int argc, char* argv[]) {
  	int status;	// wait status for child
	int shm_id;
	// ring buffer, shared memory 
  	struct SharedMemory* ringBuffer;
	
	// main process (Client process) creates shared memory
	ringBuffer = create_shared_memory(sizeof(*ringBuffer), &shm_id, 1);
	
	if (fork() == 0) { // Server, application process
		struct SharedMemory *shared_area;
  		int server_shm_id;
		double sum = 0;

		// Server creates shared memory
  		shared_area = create_shared_memory(sizeof(*shared_area), &server_shm_id, 0);
  
  		//sleep(1);
		while(1) {
			
			// buffer empty, wait for client to fill in items
			while(shared_area->head == shared_area->tail){
				// if it is "special" msg, receiving is done
				if(shared_area->buf[shared_area->head] == SEED + 1) {
					printf("Server: message Done! \n");
					break;
				}
			}				
			// if it is "special" msg, receiving is done
			if(shared_area->buf[shared_area->tail] == SEED + 1){
				break;
			}
			sum = sum + shared_area->buf[shared_area->tail];
			shared_area->tail = (shared_area->tail + 1) % BUFFER_SIZE;
		//	printf("Server: sum is %1.0f \n", sum);
			
		}
		
		// detach shared memory, if detaching fails, exit	
		if(shmdt(shared_area) == -1) {
			perror("Server: shmdt");
    			exit(1);
   		}
//   		printf("Server: shared memory detached\n");

		// Upon finish, print out the sum
		printf("Server: the sum is %1.0f \n", sum);
	} else { // client, driver process
		int i;
		double sum = 0;
		ringBuffer->head = 0;	// initialization of head and tail
		ringBuffer->tail = 0;

		// get current time before start
		double start_time = get_cur_time();
		
		// send as much as TOTAL_MSG messages.
		for(i = 0; i < TOTAL_MSG; i++) {
			// buffer full, wait for server to take out items
			while((ringBuffer->head + 1) % BUFFER_SIZE == ringBuffer->tail)
				;
			// fill in next buffer with a random integer
			ringBuffer->buf[ringBuffer->head] = gen_rand(SEED);
			// summation
			sum = sum + ringBuffer->buf[ringBuffer->head];
			// update buffer head
			ringBuffer->head = (ringBuffer->head + 1) % BUFFER_SIZE;
		}
		// get finish time
		double finish_time = get_cur_time();
		
		// send the last message
		while((ringBuffer->head + 1) % BUFFER_SIZE == ringBuffer->tail)
			;
		ringBuffer->buf[ringBuffer->head] = SEED+1;
		ringBuffer->head = (ringBuffer->head + 1) % BUFFER_SIZE;
//		printf("last message sent! \n");
		// wait Server to finish
		wait(&status);

		// detach shared memory
    		if (shmdt(ringBuffer) == -1) {
      			perror("Parent: shmdt");
      			exit(1);
    		}
    		printf("Client: shared memory detached\n");

		printf("Client: the sum is %1.0f \n", sum);

//		printf("Client: start time is:\t %f \n", start_time);
//		printf("Client: finish time is:\t %f \n", finish_time);

		// the total time used for sending
		double time_used = finish_time - start_time;
		// throughput
		int throughput = TOTAL_MSG / time_used;
		printf("Total time: %f seconds, Throughput: %d msgs / second. \n", time_used, throughput);

    		// delete shared memory
    		if (shmctl(shm_id, IPC_RMID, NULL)) {
      			perror("Client: shmctl");
      			exit(1);
    		}
	}
	return 0;
}
