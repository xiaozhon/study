/***********************************************************************
 * CSCI 503 Operating System Lab 5: Slab Memory Allocator
 * 
 * @author: Xiaoyang Zhong
 ***********************************************************************/

#include <slab_allocator.h>
#include <stdio.h>
#include <stdlib.h>
/**
 * Chop a slab into chunk list
 * @arguments:
 *		pool_node* slab: pointer to the slab
 *		int chunk_size: the size of the chunk
 * @return
 *		void*:	the pointer to the start position of the chunk list
**/

void* chop_slab(pool_node* slab, int chunk_size) {
	void* slab_position = (void*) slab;
	void* position = slab_position + sizeof(*slab);	// chunk start position
	
	while(position < slab_position + SLAB_SIZE - 1) {	// inside the slab
		if((slab_position + SLAB_SIZE - position) < sizeof(chunk_node)) {
			// slab has been fully chopped
			// the rest of the memory cannot even hold the chunk_node
			// If we break here, the result of this chop is that, the last
			// chunk node can always be formatted as a chunk_node
			break;
		}
		chunk_node* c_node = (chunk_node*) position;
		if(position == slab_position + sizeof(*slab)) {	
			// the first chunk
			c_node -> prior = c_node;
		} else {
			c_node -> prior = (chunk_node*)(position - chunk_size);
		}
		if(position + chunk_size > slab_position + SLAB_SIZE - 1) {
			// the last chunk, this chunk will not be used.
			// assume it does not exists.
			c_node -> next = NULL;
			c_node -> size = slab_position + SLAB_SIZE - position;
		} else {
			c_node -> next = (chunk_node*) (position + chunk_size);
			c_node -> size = chunk_size;
		}
		position = position + chunk_size;
	}
	return (slab_position + sizeof(*slab));	// return the start position of the chunk list
}

/**
 * Given the requested number of bytes, find the class type. 
 * If the requested size is too large, return -1.
 * @arguments:
 *		int num_bytes: the requested number of bytes
 * @return
 *		int class_type: the type of the class
**/
int check_class_type(slab_allocator * alloc, int num_bytes) {
	int i;
	for(i = 0; i < alloc -> num_classes; i++) {
		if(class_table[i].chunk_size - CHUNK_OFFSET >= num_bytes) {
			return i;
		}
	}
	return -1;
}

/**
 * Initialization of slab allocator. A memory pool is allocated to the 
 * slab allocator, when a new slab is needed, get a free slab from the pool
 *
 * The pool is formatted as a double linked list, each node is a slab.
 *
 * @argument
 *		slab_allocator *alloc: a pointer to the memory pool 
 *		int mem_pool_size:	the size of the memory pool
**/
void init_slab_allocator(slab_allocator *alloc, size_t mem_pool_size) {
	int i;
	void* memory;
	void* mem_pool;		// memory pool
	int num_slabs;		// slab number
	int free_slabs;		// number of free slabs
	int num_classes;	// number of classes
	
	num_slabs = mem_pool_size / SLAB_SIZE;	// each slab is 1 MB;
	
	if(num_slabs <= 0) {
		printf("ERROR: mem_pool_size should be larger than %d bytes\n", SLAB_SIZE);
		exit(1);
	}
	// if the mem_pool_size is not a multiple of SLAB_SIZE
	if(num_slabs * SLAB_SIZE < mem_pool_size) {
		printf("ERROR: mem_pool_size should be a multiple of %d \n", SLAB_SIZE);
		exit(1);
	}
	
	// memory region mem_pool ~ (mem_pool + mem_pool_size -1) are malloced for use.
	memory = malloc(mem_pool_size);
	mem_pool = memory;

	// set class number
	if(num_slabs > CLASS_NUM) {
		num_classes = CLASS_NUM;
		free_slabs = num_slabs - num_classes;
	} else {
		// in this case, there are actually no free slabs, all slabs are
		// chopped into chunks.
		num_classes = num_slabs;
		free_slabs = 0;
	}
	
	// build memory pool double linked list
	void* pool_position = mem_pool;
	int count = 0;	// used for debug
	while(pool_position < (mem_pool + mem_pool_size - 1)) {
		// This struture is put for the beginning of each slab
		pool_node* p_node = (pool_node*) pool_position;	
		count++;
		
		if(pool_position == mem_pool) {
			// special care for the prior of first slab	
			p_node -> prior = p_node;
	
		} else {	// other slabs
			p_node -> prior = (pool_node*) (pool_position - SLAB_SIZE);
		}
		
		if(pool_position + SLAB_SIZE > mem_pool + mem_pool_size - 1) { 
			// special care for the next of last slab
			p_node -> next = NULL;
		} else {
			p_node -> next = (pool_node*) (pool_position + SLAB_SIZE);
		}
		// move to next slab
		pool_position = pool_position + SLAB_SIZE;
	}
	
	pool_position = mem_pool;
	// int class_table[CLASS_NUM] defined in header file
	// compute the size of the chunk
	// assign the slab to the class
	int j;
	
	for(i = 0; i < num_classes; i++) {
		// compute the size of the class
		if(i == 0) {
			class_table[i].chunk_size = FIRST_CHUNK_SIZE;
		} else {
			class_table[i].chunk_size = 1.25 * class_table[i - 1].chunk_size;
			// make it as a multiple of 8
			j = class_table[i].chunk_size % 8;
			if(j > 4) {
				class_table[i].chunk_size = class_table[i].chunk_size + 8 - j;
			} else {
				class_table[i].chunk_size = class_table[i].chunk_size - j;
			}
		}
		
		class_table[i].class_type = i;
		// assign slab
		class_table[i].slab = (pool_node*)(pool_position + i * SLAB_SIZE);
		class_table[i].slab -> next = NULL;
		class_table[i].slab -> prior = class_table[i].slab;
			
		class_table[i].chunk_list = chop_slab(class_table[i].slab, class_table[i].chunk_size);
		class_table[i].free_chunks = (SLAB_SIZE - sizeof(pool_node)) / class_table[i].chunk_size;
	}
	
	alloc -> memory = memory;
	// pointer to free slabs list
	alloc -> slab_pool = mem_pool + num_classes * SLAB_SIZE;
	alloc -> num_slabs = num_slabs;
	alloc -> free_slabs = free_slabs;
	alloc -> num_classes = num_classes;
	alloc -> mem_pool_size = mem_pool_size;
} 

/**
 * Allocator memory. Find the class of slab using num_bytes, if the
 * class has free chunks, assign it; else, get a free slab in the pool,
 * assign the new slab to the class, and assign it; else, return NULL;
 *
 * @argument
 *		slab_allocator *alloc: the pointer to the memory pool
 *		int num_bytes: the size of the requested memory
 * @return 
 *		the pointer to the allocated memory chunk
**/
void *alloc_mem(slab_allocator *alloc, int num_bytes){
	int class_type = check_class_type(alloc, num_bytes);
	chunk_node* return_node;

	if(class_type < 0) {
		printf("ERROR: Requested size exceeds the maximum value %d \n", 
				class_table[CLASS_NUM - 1].chunk_size - CHUNK_OFFSET);
	//	free(alloc -> memory);
		return NULL;
	}
	if(class_type > alloc -> num_classes) {
		printf("ERROR: Requested size exceeds the maximum value %d \n", 
				class_table[alloc -> num_classes - 1].chunk_size - CHUNK_OFFSET);
	//	free(alloc -> memory);
		return NULL;
	}
	
	if(class_table[class_type].free_chunks <= 0) {
		// the class has no free chunks, get a slab from the pool
		if(alloc -> free_slabs > 0 ) {
			// the pool has free slabs, get one free slab from the 
			// header of the list
			pool_node* new_slab = (pool_node*)alloc -> slab_pool;
			
			alloc -> slab_pool = (void*) new_slab -> next;
			if(new_slab -> next != NULL) {
				// this is not the last slab
				pool_node* next_slab = (pool_node*) new_slab -> next;
				next_slab -> prior = next_slab;
			}
			void* new_chunks = chop_slab(new_slab, class_table[class_type].chunk_size);
			
			class_table[class_type].chunk_list = new_chunks;
			class_table[class_type].free_chunks = (SLAB_SIZE - sizeof(pool_node)) / class_table[class_type].chunk_size;
			
			alloc -> free_slabs--;
			
		} else {
			// no free slabs, no free chunks, return NULL;
			printf("(alloc_mem) ERROR: No avaliable memory! \n");
		//	free(alloc -> memory);
			return NULL;
		}
	} 
	
//	else{
		// the class has free chunks
		// get the first chunk from the list
	return_node = (chunk_node*)class_table[class_type].chunk_list;
	
	class_table[class_type].chunk_list = (void*)return_node -> next;
	
	if(return_node -> next != NULL){
		// if this is not the last chunk
		chunk_node* next_node = (chunk_node*)return_node -> next;
		next_node -> prior = next_node;
	}
	class_table[class_type].free_chunks--;
	
	return ((void*)return_node) + CHUNK_OFFSET;
//	}
	
}

/**
 * Free memory. 
 *
 * @argument
 *		slab_allocator *alloc: the pointer to the memory pool
 *		void* ptr: pointer to the memory slot that to be freed.
 * @return 
 *		0: if freed SUCCESS
 *		-1: if FAIL
**/
int free_mem(slab_allocator *alloc, void* ptr){
	if(ptr > alloc -> memory + alloc -> mem_pool_size || ptr < alloc -> memory) {
		printf("ERROR: the pointer ptr is not in the memory pool!\n");
		return -1;
	}
	chunk_node* f_node = (chunk_node*) (ptr - CHUNK_OFFSET);
//	int class_type = check_class_type(f_node -> size);
	int class_type = -1;
	int i;
	for(i = 0; i < alloc -> num_classes; i++) {
		if(class_table[i].chunk_size == f_node -> size) {
			class_type = i;
			break;
		}
	}
	if(i == alloc -> num_classes) {
		printf("ERROR: Wrong memory location to be freed!\n");
		return -1;
	}

	if(class_table[class_type].chunk_list == NULL) {
		// the chunk list is empty now
		class_table[class_type].chunk_list = f_node;
		f_node -> next = NULL;
		f_node -> prior = f_node;
		class_table[class_type].free_chunks++;
		return 0;
	} else {
		// insert to the head of the list
		chunk_node* chunk_head = (chunk_node*) class_table[class_type].chunk_list;
		class_table[class_type].chunk_list = (void*) f_node;
		f_node -> next = chunk_head;
		f_node -> prior = f_node;
		chunk_head -> prior = f_node;
		
		class_table[class_type].free_chunks++;

		return 0;
	}
	
}

