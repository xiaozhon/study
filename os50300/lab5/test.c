#include <stdio.h>
#include <stdlib.h>
#include <slab_allocator.h>
#include <test.h>

void list_add(list_node* list, list_node* new_node){
	list_node* tmp = list;
	while(tmp -> next != NULL) {
		tmp = tmp -> next;
	}
	tmp -> next = new_node;
	new_node -> prior = tmp;
	new_node -> next = NULL;
	printf("		Main add: add new node %d to list, new node position: %d \n", new_node -> id, (int)new_node);
}

void list_delete_first(slab_allocator* alloc, list_node* list) {
	list_node* tmp = list;
	list_node* target_node = tmp -> next;
	
	if(tmp -> next != NULL) {
		// there are some node in the list
		printf("		Main delete: delete node id: %d, node position: %d \n", target_node -> id, (int)target_node);
		tmp -> next = target_node -> next;
		list_node* next_node = target_node -> next;
		if(target_node -> next != NULL) {
			next_node -> prior = tmp;
		}
		free_mem(alloc, target_node);
	} else {
		// do nothing, all nodes are deleted
		printf("		Main delete: all nodes are deleted!\n");
	}
}

int main(int argc, char* argv[]) {
	slab_allocator alloc;
	size_t mem_pool_size;
	
	int retval;
	int i;
	
	if(argc != 2) {
		printf("Usage: %s mem_pool_size\n", argv[0]);
		exit(1);
	}
	mem_pool_size = atoi(argv[1]);
	
	printf("pool_node size is: %d \n", sizeof(pool_node));
	init_slab_allocator(&alloc, mem_pool_size);
//	list_node* list;
//	list = 

	int* m = (int*) alloc_mem(&alloc, sizeof(int));
	printf("Main: Address of int* m is: %d \n", (int)m);
	
	if((retval = free_mem(&alloc, m)) < 0) {
		printf("Main: free mem fail\n");
	}
	
	list_node* list = (list_node*)alloc_mem(&alloc, sizeof(*list));
	if(list == NULL){
		printf("Main: alloc fail!\n");
		return 1;
	}
	list -> prior = list;
	list -> id = 0;
	list -> next = NULL;
	for(i = 1; i < 10; i++){
		list_node* new_node = (list_node*)alloc_mem(&alloc, sizeof(*new_node));
		if(new_node == NULL){
			printf("Main: alloc fail!\n");
			break;
		}
		new_node -> id = i;
		printf("	Main for: new node %d, new node pointer position %d\n", new_node -> id, (int)new_node);
		list_add(list, new_node);
	}
	
	for(i = 1; i < 10; i++){
		list_delete_first(&alloc, list);
	}
	
	int j;
	int c[12];
	c[0] = 80;
	for(i = 1; i < 12; i++) {
		c[i] = 1.25 * c[i - 1];
	//	c[i] = c[i] / 8;
	//	c[i] = c[i] * 8;
		j = c[i] % 8;
		if(j > 4)
			c[i] = c[i] + (8 - j);
		else
			c[i] = c[i] - j;
//		printf("i is: %d, c[i] is: %d \n", i, c[i]);
	}
	
	void* test[100];
	for(j = 0; j < 20; j++) {
		test[j] = alloc_mem(&alloc, c[0]);
		printf("Main: allocate memory from class 1, test[%d] position is: %d\n", j, (int)test[j]);
	}
	
//	test[0] = malloc(10);
//	free_mem(&alloc, test[0]);
	
	
	free(alloc.memory);
	return 1;
}