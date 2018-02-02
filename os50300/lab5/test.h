/**
 * Test file for slab allocator
**/

typedef struct test_list_t{
	int id;
	struct list_node* next;
	struct list_node* prior;
}list_node;