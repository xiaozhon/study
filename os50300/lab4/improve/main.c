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
//	printf("	list_add: before while \n");
	while(tmp -> next != NULL){
//		printf("	list_add: in while \n");
		tmp = tmp -> next;
	}
//	printf("	list_add: out of while \n");
	tmp -> next = node;	
//	printf("test 1\n");
	node -> prior = tmp;
//	printf("test 2\n");
	node -> next = NULL;
//	printf("test 3\n");
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
 * list operations: delete the "node" in the list
 * 
 * @argument
 *		Person: the list
 *		Person: pointer to the node
 * @return
 *		Person: the pointer to prior node
 *				
**/
Person list_delete(Person list, Person node) {
	Person rt = node -> prior;
	if(node -> next == NULL) {
		// node is the last in the list
		node -> prior -> next = NULL;
		free(node);
		
	} else {
		// node is in the middle of the list
		node -> prior -> next = node -> next;
		node -> next -> prior = node -> prior;
		free(node);
	}
	return rt;
}

/**
 * Initialization of threads
**/
void init_lv(LV lv, int tid, GV gv, FP* floor_persons, EL el){
	lv -> tid = tid;
	lv -> gv = gv;
	lv -> floor_persons = floor_persons;
	if(tid != 0){
		// initialization of elevator
		el -> id = lv -> tid;
		el -> current_floor = 1;
		el -> dir = UP;
		Person tmp = (Person) malloc(sizeof(*tmp));
		tmp -> prior = tmp; 
		tmp -> next = NULL;
		
		el -> people = tmp;
		
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
	FP* floor_person_list;
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
	// initialization of floor person list
	floor_person_list = (FP*) malloc(sizeof(*floor_person_list) * (n_floors + 1));

	for(i = 0; i < n_floors + 1; i++) {
		floor_person_list[i] = (FP) malloc(sizeof(*floor_person_list[i]));
	
		floor_person_list[i] -> count = 0;
		pthread_mutex_init(&floor_person_list[i] -> floor_lock, NULL);
		Person tmp = (Person) malloc(sizeof(*tmp));
		
		tmp -> prior = tmp; 
		tmp -> next = NULL;
		floor_person_list[i] -> per_floor_list = tmp;
	}
	
	// create elevator threads
	for(i = 0; i <= n_elevators; i++) {
		// if i = 0, it is people generator thread
		// else, it is elevator thread
		init_lv(lvs + i, i, gv, floor_person_list, elevators + i);
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
	
	for(i = 0; i < n_floors + 1; i++) {
		if((rc = pthread_mutex_destroy(&floor_person_list[i] -> floor_lock)) != 0) {
			perror("pthread_mutex_destory_floor_lock()");
		}
		free(floor_person_list[i] -> per_floor_list);
	}
	free(floor_person_list);
	free(gv);
	free(elevators);
	free(lvs);
	free(attrs);
	free(thrds);
	
}
