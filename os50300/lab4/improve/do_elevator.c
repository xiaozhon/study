#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "elevator.h"

void * do_elevator(void *v) {
	LV lv;
	GV gv;
//	Person person_list;		// we may not need this people list
	EL elevator;
	FP* floor_persons;

	double cur_time;
	Person customer;	// the customer of the elevator
	
	lv = (LV)v;
	gv = (GV) lv -> gv;
//	person_list = (Person) lv -> p_list;
	elevator = (EL) lv -> el;
	floor_persons = (FP*) lv -> floor_persons;
	
//	printf("Elevator: elevator %d started \n", elevator -> id);
	
	cur_time = get_cur_time();
	while((cur_time - gv -> start_time) < gv -> simulation_time) {
		
		// arrive at a floor
		printf("[%f] Elevator %d arrives at %d \n", get_cur_time() - gv -> start_time, 
					elevator -> id, elevator -> current_floor);
	
		// check the person in this floor (current_floor)
		pthread_mutex_lock(&floor_persons[elevator -> current_floor] -> floor_lock);
		if(floor_persons[elevator -> current_floor] -> count > 0) {
			// there are some people in this floor
			
			customer = floor_persons[elevator -> current_floor] -> per_floor_list;
			while(customer -> next != NULL) {
				// the head of the list is an empty node
				customer = customer -> next;
				if(customer -> dir == elevator -> dir) {
					// pick up this person
					// TODO: modify the list
					Person new_customer;
					new_customer = (Person) malloc(sizeof(*new_customer));
					new_customer -> id = customer -> id;
					new_customer -> from_floor = customer -> from_floor;
					new_customer -> to_floor = customer -> to_floor;
					new_customer -> dir = customer -> dir;
					new_customer -> arrival_time = customer -> arrival_time;
				
					printf("[%f] Elevator %d picks up Person %d \n", 
						(get_cur_time() - gv -> start_time), elevator -> id, new_customer -> id);
					list_add(elevator -> people, new_customer);
					//printf("test7 \n");
					
					customer = list_delete(floor_persons[elevator -> current_floor] -> per_floor_list, customer);
					floor_persons[elevator -> current_floor] -> count--;
					
					gv -> num_people_started++;	
				} else {
					// do nothing
				}
			}
		
		} else {// no person is in this floor, do nothing
		
		}
		pthread_mutex_unlock(&floor_persons[elevator -> current_floor] -> floor_lock);

		// check elevator's list, if there are somebody wants to go
		// this floor, drop it
		customer = elevator -> people;
		while(customer -> next != NULL) {
			customer = customer -> next;
			if(customer-> to_floor == elevator -> current_floor) {
				// drop the person, delete the node
				// TODO: modify the list
				printf("[%f] Elevator %d drops Person %d \n", 
						get_cur_time() - gv -> start_time, elevator -> id, customer -> id);
				customer = list_delete(elevator -> people, customer);	
				
				pthread_mutex_lock(&gv -> st_lock);
				gv -> num_people_finished++;	
				pthread_mutex_unlock(&gv -> st_lock);
			} else {
				// do nothing
			}
		}
	
		// keep moving
		if(elevator -> dir == UP) {
			printf("[%f] Elevator %d starts moving from %d to %d \n", 
					(get_cur_time() - gv -> start_time), elevator -> id, 
					elevator -> current_floor, elevator -> current_floor + 1);
		} else {
			printf("[%f] Elevator %d starts moving from %d to %d \n", 
					(get_cur_time() - gv -> start_time), elevator -> id, 
					elevator -> current_floor, elevator -> current_floor - 1);
		}
		
		sleep(gv -> elevator_speed);    
	
		if(elevator -> dir == UP) {
			elevator -> current_floor++;
			if(elevator -> current_floor == gv -> num_floors) {
				elevator -> dir = DOWN;
			}
		} else {
			elevator -> current_floor--;
			if(elevator -> current_floor == 1) {
				elevator -> dir = UP;
			}
		}
		
		cur_time = get_cur_time();
		
		//	printf("	Elevator %d: person information: \n", elevator -> id);
	//	printf("	Elevator %d:	id: %d \n", elevator -> id,customer -> id );
	//	printf("	Elevator %d:	from: %d \n", elevator -> id,customer -> from_floor );
	//	printf("	Elevator %d:	to: %d \n", elevator -> id,customer -> to_floor );
	//	printf("	Elevator %d:	dir: %d \n", elevator -> id,customer -> dir_up );
	//	printf("	Elevator %d:	arrival_time: %f \n", elevator -> id,customer -> arrival_time );

		
	}
}
