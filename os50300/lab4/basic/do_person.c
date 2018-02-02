#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "elevator.h"



void * do_person(void *v) {
	LV lv;
	GV gv;
	Person person_list;
	
//	int i;
	int result;	// the result of pthread_cond_broadcast
	int p_id = 1;
	double cur_time;
	lv = (LV)v;
	gv = (GV) lv -> gv;
	person_list = (Person) lv -> p_list;
	
//	printf("Person: people generator starts\n");
	
	while(1) {
		cur_time = get_cur_time() - gv -> start_time;
	//	printf("Person: cur time is %f \n", cur_time);
		
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
//		printf("Person: about to generate a new person \n");
		// generate a person, with random from floor, random dest floor
		Person new_person;
		new_person = (Person) malloc(sizeof(*new_person));
//		printf(".............Person: new Person generated\n");
		new_person -> id = p_id++;
		new_person -> from_floor = (random() % gv -> num_floors) + 1;
		new_person -> to_floor = (random() % gv -> num_floors) + 1;
		// if the random from floor and dest floor is the same, generate a new random
		// dest floor
		while(new_person -> to_floor == new_person -> from_floor)
			new_person -> to_floor = (random() % gv -> num_floors) + 1;
		// new person is going down
		if(new_person -> from_floor > new_person -> to_floor) {
			new_person -> dir_up = 0;
//			printf("		Person: person %d is going down \n", new_person -> id);
		}	
		else {
			// new person is going up
			new_person -> dir_up = 1;
//			printf("		Person: person %d is going up \n", new_person -> id);

		}
		// arrival time since the simulation is started
		new_person -> arrival_time = get_cur_time() - gv -> start_time;
		
		printf("[%f] Person %d arrives on floor %d, waiting to go to floor %d \n", 
				new_person -> arrival_time, new_person -> id, new_person -> from_floor, new_person -> to_floor);	

	//	printf("	Person: waiting the p lock\n");
		// get lock to person list
		pthread_mutex_lock(&gv -> p_lock);	
	//	printf("	Person: gets the p lock\n");
		// add person to list
//		printf("................Person: Add person to list\n");
		list_add(person_list, new_person);
		
		gv -> personCount++;
	//	printf("	Person: total person is: %d \n", gv -> personCount);
		// release the lock
		pthread_mutex_unlock(&gv -> p_lock);
	//	printf("	Person: release the p lock \n");
		
		// signal all other threads that are waiting on a empty list
	//	if(gv -> personCount == 1) {
//		printf("................Person: signal all other threads\n");

		if((result = pthread_cond_broadcast(&gv -> p_cv)) != 0) {
			perror("cond broadcast");
		}
	//	}

	}
	
}
