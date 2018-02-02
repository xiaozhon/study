#include <pthread.h>

/**
 * Data structure:
 *		person;		// single person
 *		double list,	// person list
 *		elevator,
 *		gv,				// globle variables
 * 		thrd_t			// thread informations
 *
 
**/
typedef struct thrd_t {
	int	tid;	// tid starts from 1;
	void *gv;	// shared memory
	void *p_list;
	void *el;
}* LV;


typedef struct gv_t {
	int num_elevators;
    int num_floors;
    int arrival_time;
    int elevator_speed;
    int simulation_time;
    int random_seed;
	
    int num_people_started;		// statistics
    int num_people_finished;	
    pthread_mutex_t st_lock;		// lock for statistics
	
	int personCount;
	pthread_mutex_t p_lock;	// lock to access person list, and person count
	pthread_cond_t p_cv;
	
	double start_time;
}* GV;

typedef struct person {
	int id;		// person id starts from 1
    int from_floor, to_floor;	// floors start from 1 to num_floors
	int dir_up;		// if direction is up, set to 1, else, set to 0
    double arrival_time;
	
	struct person *next;
	struct person *prior;
}person, *Person;

typedef struct elevator {
    int id;				// elevator id is the same as thread id
    int current_floor;
//    pthread_mutex_t el_lock;	// if list empty, wait
//    pthread_cond_t el_cv;
//    Person people;    // all the person inside the elevator
								// we don't need this in basic version
}* EL;


void* do_elevator(void* arg);
void* do_person(void* arg);

// list operations
void list_add(Person list, Person node);
Person list_check_first(Person list);
void list_delete_first(Person list);

// get current time
double get_cur_time();


