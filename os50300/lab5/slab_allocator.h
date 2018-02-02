/***********************************************************************
 * CSCI 503 Operating System Lab 5: Slab Memory Allocator
 * 
 * @author: Xiaoyang Zhong
 ***********************************************************************/
/**
 * Since the free_mem function does not specify the size of the chunk,
 * in order to find the position of the chunk, store the chunk_size in 
 * the header of each chunk. Thus, the actual_chunk_size = 
 * chunk_size - 8. The first four bytes (sizeof(int)) stores the size of
 * the chunk, but we minus 8 to make the return address as a multiple of 8.
 **/ 

/**
 *	Data Structures:
 *		slab_allocator:		// pre-allocated memory pool and general 
 *							// information
 *		chunk_node:	// single chunk structure
 *		class_table
 **/
#include <stddef.h>
#define CLASS_NUM 43
#define SLAB_SIZE 1000000
#define FIRST_CHUNK_SIZE 80
#define CHUNK_OFFSET 8
typedef struct slab_allocator_t {
	void* memory;
	void* slab_pool;	// pointer to the slab pool
						// slab pool is a double linked list of slabs
	int mem_pool_size;
	int num_slabs;
	int free_slabs;
	int num_classes;
}slab_allocator;

typedef struct pool_node_t {// each node is a slab
	struct pool_node* prior;
	struct pool_node* next;
}pool_node;

typedef struct class_table_t {	// assign slabs to classes
	pool_node* slab;		// slab for this class 	
	void* chunk_list;
	int class_type;		// class type
	int chunk_size;		// the size of the chunk
	int free_chunks;
}Classes;

typedef struct chunk_node_t {	// each node points to a chunk
	int size;
	struct chunk_node* next;
	struct chunk_node* prior;
}chunk_node;

Classes class_table[CLASS_NUM];

// slab allocator functions
void *alloc_mem(slab_allocator *alloc, int num_bytes);
int free_mem(slab_allocator *alloc, void* ptr);
void init_slab_allocator(slab_allocator *alloc, size_t mem_pool_size);