#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "elevator.h"

/**
 * Initialization of elevator information
 * 
**/
void init_elevator(LV lv, EL elevator) {
	elevator -> id = lv -> tid;
	elevator -> current_floor = 1;
	
//	pthread_mutex_init(&elevator -> el_lock, NULL);
//	pthread_cond_init(&elevator -> el_cv, NULL);
	
  //  elevator -> people = NULL; 
}

void * do_elevator(void *v) {
	LV lv;
	GV gv;
	Person person_list;
	EL elevator;
	
	int target_id;
	int target_floor;
	int sleep_t;
	double cur_time;
	int moving_time;	// the time needed for the elevator to move the customer
	Person customer;	// the customer of the elevator
	
	lv = (LV)v;
	gv = (GV) lv -> gv;
	person_list = (Person) lv -> p_list;
	elevator = (EL) lv -> el;
	
	init_elevator(lv, elevator);
	
//	printf("Elevator: elevator %d started \n", elevator -> id);
	
	cur_time = get_cur_time();
	while((cur_time - gv -> start_time) < gv -> simulation_time) {
	//	pthread_mutex_lock(&elevator -> el_lock);
		// if the person list is empty, block
		pthread_mutex_lock(&gv -> p_lock);
		while (gv -> personCount <= 0) {
		//	printf("Elevator: No People ! \n");
	//		pthread_cond_wait(&elevator -> el_cv, &elevator -> el_lock);
			pthread_cond_wait(&gv -> p_cv, &gv -> p_lock);
		}		
		// the list is not empty, check the first person from the list, 
		// do not remove it yet
		customer = list_check_first(person_list);
		target_id = customer -> id;
		target_floor = customer -> from_floor;
		
		pthread_mutex_unlock(&gv -> p_lock);
		if(customer == NULL) {
			cur_time = get_cur_time();
			continue;
		}
		
	//	printf("	Elevator %d: person information: \n", elevator -> id);
	//	printf("	Elevator %d:	id: %d \n", elevator -> id,customer -> id );
	//	printf("	Elevator %d:	from: %d \n", elevator -> id,customer -> from_floor );
	//	printf("	Elevator %d:	to: %d \n", elevator -> id,customer -> to_floor );
	//	printf("	Elevator %d:	dir: %d \n", elevator -> id,customer -> dir_up );
	//	printf("	Elevator %d:	arrival_time: %f \n", elevator -> id,customer -> arrival_time );

		// move to the customer's current floor
		if(elevator -> current_floor > target_floor){
			// moving down
			printf("[%f] Elevator %d starts moving from %d to %d \n", 
					(get_cur_time() - gv -> start_time), elevator -> id, 
					elevator -> current_floor, target_floor);
			sleep_t = (elevator -> current_floor - target_floor) * gv -> elevator_speed;
			elevator -> current_floor = target_floor;

	//		printf("Elevator: Elevator %d sleep time is: %d \n", elevator -> id, sleep_t);
			
		//	sleep((elevator -> current_floor - customer -> from_floor ) * gv -> elevator_speed);
			sleep(sleep_t);

		} else  if(elevator -> current_floor < target_floor){

			// moving up
			printf("[%f] Elevator %d starts moving from %d to %d \n", 
					(get_cur_time() - gv -> start_time), elevator -> id, 
					elevator -> current_floor, target_floor);
			sleep_t = (target_floor - elevator -> current_floor) * gv -> elevator_speed;
	//		printf("Elevator: Elevator %d sleep time is: %d \n", elevator -> id, sleep_t);
		
			elevator -> current_floor = target_floor;

		//	sleep(( customer -> from_floor - elevator -> current_floor) * gv -> elevator_speed);
			sleep(sleep_t);
		} else {
			// on the same floor, pick up the customer
		}
		// reach the customer's current floor
//		elevator -> current_floor = customer -> from_floor;
		cur_time = get_cur_time();
		if((cur_time - gv -> start_time) > gv -> simulation_time) {
	//		printf("	Elevator %d simulation done\n", elevator -> id);
			break;
		}
		printf("[%f] Elevator %d arrives at %d \n", get_cur_time() - gv -> start_time, 
					elevator -> id, elevator -> current_floor);
		
		// when arrive, the simulation is done
		
		
//		printf("	Elevator: elevator %d waiting p lock \n", elevator -> id);
		pthread_mutex_lock(&gv -> p_lock);
	//	printf("	Elevator: elevator %d gets p lock \n", elevator -> id);
		customer = list_check_first(person_list);
		if(customer == NULL) {
//			printf("	Elevator %d: continue\n ", elevator -> id);
			cur_time = get_cur_time();
			pthread_mutex_unlock(&gv -> p_lock);
//			printf("	Elevator: elevator %d release p lock \n", elevator -> id);
			continue;
		}
					
		if(customer -> id == target_id || customer -> from_floor == elevator -> current_floor) {
			// if the customer is still there, or there is another customer there, pick up
			target_id = customer -> id;
			printf("[%f] Elevator %d picks up Person %d \n", 
					(get_cur_time() - gv -> start_time), elevator -> id, customer -> id);
			
			// move the customer to destination
			if(customer -> dir_up){
				// customer is moving up
				moving_time = (customer -> to_floor - customer -> from_floor ) * gv -> elevator_speed;
//				printf("	Elevator: elevator %d moves person up \n", elevator -> id);	
//				printf("	Elevator: elevator %d moving time %d \n", elevator -> id, moving_time);

			} else {
				// customer is moving down
				moving_time = (customer -> from_floor - customer -> to_floor ) * gv -> elevator_speed;
//				printf("	Elevator: elevator %d moves person down \n", elevator -> id);
//				printf("	Elevator: elevator %d moving time %d \n", elevator -> id, moving_time);
			}
			elevator -> current_floor = customer -> to_floor;
		
//			printf("	Elevator %d waiting for st lock\n", elevator -> id);
			// count the person that started
			pthread_mutex_lock(&gv -> st_lock);
//			printf("	Elevator %d gets st lock\n", elevator -> id);
			gv -> num_people_started++;	
//			printf("	Elevator %d: started ++ \n", elevator -> id);
			pthread_mutex_unlock(&gv -> st_lock);
//			printf("	Elevator %d release st lock \n", elevator -> id);
			// move to the customer's dest floor
		
			list_delete_first(person_list);
			gv -> personCount--;
		
			// release the lock
			pthread_mutex_unlock(&gv -> p_lock);
//			printf("	Elevator %d release p lock\n", elevator -> id);
		
			cur_time = get_cur_time();
		
			if((gv -> simulation_time - (cur_time - gv -> start_time)) > moving_time) {
				// count the person that can finish
				sleep(moving_time);
				printf("[%f] Elevator %d drops Person %d \n", 
					get_cur_time() - gv -> start_time, elevator -> id, target_id);
				// count the person that finished
				
	//			printf("	Elevator %d waiting for st lock for finish\n", elevator -> id);
				pthread_mutex_lock(&gv -> st_lock);
				gv -> num_people_finished++;	
	//			printf("	Elevator %d fihished\n", elevator -> id);
				pthread_mutex_unlock(&gv -> st_lock);
			} else {
				sleep(moving_time);
	//			printf("	Elevator: elevator %d do not finish moving \n", elevator -> id);
			}
			
		} else {
			// else, release the lock, next run
			
			pthread_mutex_unlock(&gv -> p_lock);
		}
		cur_time = get_cur_time();
	}

//	printf("Elevator %d terminates \n", elevator -> id);

}

