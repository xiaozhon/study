#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "elevator.h"

void * do_person(void *v) {
	LV lv;
	GV gv;
	FP* floor_persons;
	
//	int i;
	int result;	// the result of pthread_cond_broadcast
	int p_id = 1;
	double cur_time;
	lv = (LV)v;
	gv = (GV) lv -> gv;
	floor_persons = (FP*) lv -> floor_persons;;
	
//	printf("Person: people generator starts\n");
	
	while(1) {
		cur_time = get_cur_time() - gv -> start_time;
		
		sleep(gv -> arrival_time);	// sleep p seconds
		
		// simulation times up
		if((gv -> simulation_time - cur_time) < 0) {
			// signal all threads that are waiting
			if((result = pthread_cond_broadcast(&gv -> p_cv)) != 0) {
				perror("cond broadcast");
			}
	//		printf("Person: terminate\n");
			break;
		}
		
		// generate a person, with random from floor, random dest floor
		Person new_person;
		new_person = (Person) malloc(sizeof(*new_person));
		new_person -> id = p_id++;
		new_person -> from_floor = (random() % gv -> num_floors) + 1;
		new_person -> to_floor = (random() % gv -> num_floors) + 1;
		// if the random from floor and dest floor is the same, generate a new random
		// dest floor
		while(new_person -> to_floor == new_person -> from_floor)
			new_person -> to_floor = (random() % gv -> num_floors) + 1;
		// new person is going down
		if(new_person -> from_floor > new_person -> to_floor) {
			new_person -> dir = DOWN;
		}	
		else {
			// new person is going up
			new_person -> dir = UP;
		}
		// arrival time since the simulation is started
		new_person -> arrival_time = get_cur_time() - gv -> start_time;
		
		printf("[%f] Person %d arrives on floor %d, waiting to go to floor %d \n", 
				new_person -> arrival_time, new_person -> id, new_person -> from_floor, new_person -> to_floor);	

		// get lock to person list
		pthread_mutex_lock(&floor_persons[new_person -> from_floor] -> floor_lock);

		// add person to list
		list_add(floor_persons[new_person -> from_floor] -> per_floor_list, new_person);
		floor_persons[new_person -> from_floor] -> count++;
		// release the lock
		pthread_mutex_unlock(&floor_persons[new_person -> from_floor] -> floor_lock);


	}
	
}
