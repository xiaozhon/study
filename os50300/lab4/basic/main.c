#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include <elevator.h>

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

  gettimeofday(&tv, &tz);
  cur_time = tv.tv_sec + tv.tv_usec / 1000000.0;

  return cur_time;
}

/**
 * list operations: add a node to the end of the list
 * 
 * @argument
 *		Person: the list
 *		Person: the node
**/
void list_add(Person list, Person node) {
	// reach the list end
	Person tmp = list;
	while(tmp -> next != NULL){
		tmp = tmp -> next;
	}
	tmp -> next = node;
	node -> prior = tmp;
	node -> next = NULL;
}

/**
 * list operations: check the first node in the list
 * 
 * @argument
 *		Person: the list
 * @return
 *		poiter to the first node in the list
**/
Person list_check_first(Person list){
	return list -> next;
}

/**
 * list operations: delete the first node in the list
 * 
 * @argument
 *		Person: the list
 *		Person: the node
**/
void list_delete_first(Person list) {
	if(list -> next != NULL){	// there is at least one node in the list
		Person tmp = list -> next;
		list -> next = tmp -> next;
		if(tmp -> next != NULL) {
			tmp -> next -> prior = list;
		}
		free(tmp);
	} 
}

/**
 * Initialization of people generator thread
**/
void init_lv(LV lv, int tid, GV gv, Person p_list, EL el){
	lv -> tid = tid;
	lv -> gv = gv;
	lv -> p_list = p_list;
	if(tid != 0){
		lv -> el = el;
	} else {
		lv -> el = NULL;
	}
	
}


/**
 * Main function, initialize all data structures, create producer and consumer threads
**/
int main(int argc, char* argv[]) {
	int i;
	int n_elevators;
	int n_floors;
	int arrive_rate;
	int elevator_speed;
	int total_time;
	int random_seed;
		
	if(argc != 7) {
		fprintf(stderr, "Usage: %s num_elevators num_floors arrive_rate(second) ", argv[0]);
		fprintf(stderr, "elevator_speed simulation_time random_seed \n");
		exit(1);
	}
	
	// get arguments
	n_elevators 	= atoi(argv[1]);
	n_floors 		= atoi(argv[2]);
	arrive_rate		= atoi(argv[3]);
	elevator_speed	= atoi(argv[4]);
	total_time 		= atoi(argv[5]);
	random_seed 	= atoi(argv[6]);
	
	if(n_elevators <= 0) {
		fprintf(stderr, "The number of elevators should be greater than 0 \n");
		exit(1);
	}

	if(n_floors <= 0) {
		fprintf(stderr, "The number of floors should be greater than 0 \n");
		exit(1);
    }

	if(arrive_rate <= 0) {
		fprintf(stderr, "People arrive time should be greater than 0 \n");
		exit(1);
	}
	if(elevator_speed < 0) {
		fprintf(stderr, "Elevator speed should be greater than 0 \n");
		exit(1);
	}
	
	if(total_time <= 0) {
		fprintf(stderr, "Total simulation time should be greater than 0 \n");
		exit(1);
	}
	
//	if(random_seed < 0) {
//		fprintf(stderr, "Seed for random number generator should not be nagetive \n");
//		exit(1);
//	}

	srand(random_seed);
	
	LV lvs;
	GV gv;
	pthread_t *thrds;
	pthread_attr_t * attrs;
	
	// person list
	Person person_list;
	EL elevators;
	
	void *retval;
	int join_result;
	
	gv = (GV)malloc(sizeof(*gv));
	lvs = (LV)malloc(sizeof(*lvs)*(n_elevators + 1));
	elevators = (EL)malloc(sizeof(*elevators)*(n_elevators + 1));
	
	thrds = (pthread_t*) malloc(sizeof(pthread_t)*(n_elevators + 1));
	attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*(n_elevators + 1));
	// initialization of GV
	gv -> num_elevators = n_elevators;
    gv -> num_floors = n_floors;
    gv -> arrival_time = arrive_rate;
    gv -> elevator_speed = elevator_speed;
    gv -> simulation_time = total_time;
    gv -> random_seed = random_seed;
	
    gv -> num_people_started = 0;		// statistics
    gv -> num_people_finished = 0;	
	
	gv -> personCount = 0;
	
	pthread_mutex_init(&gv -> st_lock, NULL);
	pthread_mutex_init(&gv -> p_lock, NULL);
	pthread_cond_init(&gv -> p_cv, NULL);
	
	gv -> start_time = get_cur_time();
	
//	printf("Main: start time is: %f \n", gv -> start_time);
	// initialization of person list
	person_list = (Person)malloc(sizeof(*person_list));
	person_list -> prior = person_list; 
	person_list -> next = NULL;
	
	// create elevator threads
	for(i = 0; i <= n_elevators; i++) {
		// if i = 0, it is people generator thread
		// else, it is elevator thread
		init_lv(lvs + i, i, gv, person_list, elevators + i);
		if(pthread_attr_init(attrs + i)) 
			perror("attr_init()");
		if(pthread_attr_setscope(attrs + i, PTHREAD_SCOPE_SYSTEM)) 
			perror("attr_setscope()");
		if(i == 0) {
			if(pthread_create(thrds + i, attrs + i, do_person, lvs+i)) {
				perror("pthread_create_person()");
				exit(1);
			}
		} else {
			if(pthread_create(thrds + i, attrs + i, do_elevator, lvs+i)) {
				perror("pthread_create_elevator()");
				exit(1);
			}
		}
		
	}
//	printf("Main: main created all threads! \n");
	// join, wait threads to terminate
	for(i = 0; i <= n_elevators; i++) {
		if((join_result = pthread_join(thrds[i], &retval)) != 0){
			perror("pthread_join()");
		} 
	}
	printf("Simulation result: %d people have started,", gv -> num_people_started); 
	printf( "%d people have finished during %d seconds \n",
			gv -> num_people_finished, gv -> simulation_time);
	
	int rc;
	if((rc = pthread_mutex_destroy(&gv -> st_lock)) != 0) {
		perror("pthread_mutex_destory_st_lock()");
	}
	if((rc = pthread_mutex_destroy(&gv -> p_lock)) != 0) {
		perror("pthread_mutex_destory_p_lock()");
	}
	if((rc = pthread_cond_destroy(&gv -> p_cv)) != 0) {
		perror("pthread_cond_destory_p_cv()");
	}
	
	free(gv);
	free(elevators);
	free(lvs);
	free(attrs);
	free(thrds);
	free(person_list);

}
