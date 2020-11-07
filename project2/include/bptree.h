// bptree.h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "file.h"
#define LEAF_ORDER 32 
#define INTERNAL_ORDER 249 

struct key_value_t{
	int64_t key; // 8byte
	char value[120]; // 120byte
};	
typedef struct key_value_t key_value_t;

struct key_pagenum_t{
	int64_t key; // 8byte
	pagenum_t pagenum; // 8byte
};
typedef struct key_pagenum_t key_pagenum_t;

struct header_page_t{
	int64_t free_start; // [0-7]
	int64_t root_start; // [8-15]
	int64_t num_pages; // [16-23]
	char reserved[4072];
};
typedef struct header_page_t header_page_t;

struct internal_page_t{
	int64_t parent_pagenum; // [0-7]
	int is_leaf; // [8-11]
	int num_keys; // [12-15]
	int64_t right_sib_pagenum; // [16-23]
	char reserved[96];
	pagenum_t one_more_pagenum;
	key_pagenum_t key_pagenum[248];
};
typedef struct internal_page_t internal_page_t;

struct leaf_page_t{
	int64_t parent_pagenum; // [0-7]
	int is_leaf; // [8-11]
	int num_keys; // [12-15]
	char reserved[104];
	int64_t right_sib_pagenum; // [120-127]
	key_value_t key_value[31];
};
typedef struct leaf_page_t leaf_page_t;

// set header
void set_header();
void set_free_pages();

// print tree
void print_tree(); 
void enqueue(pagenum_t pagenum, pagenum_t * queue, int * head, int * tail); 
pagenum_t dequeue(pagenum_t * queue, int * head, int * tail); 
int path_to_root( pagenum_t root_pagenum, pagenum_t child_pagenum ); 

// Find.
int find_leaf(int64_t key, leaf_page_t * leaf, pagenum_t * pagenum);
int find(int64_t key, char * ret);


// Insertion.
int insert(int64_t key, char * value );
int start_new_tree(int64_t key, char * value);
int insert_into_leaf( leaf_page_t * leaf, int64_t key, char * value, pagenum_t * pagenum );
int insert_into_leaf_after_splitting(leaf_page_t * leaf, 
	int64_t key, char * value, pagenum_t parent_pagenum, pagenum_t pagenum); 
int make_leaf(leaf_page_t * leaf, pagenum_t parent_pagenum, pagenum_t * new_pagenum); 
int make_internal(internal_page_t * internal, pagenum_t parent_pagenum, pagenum_t * new_pagenum);
int cut( int length ); 
int insert_into_parent(page_t * left, int64_t key, page_t * right, pagenum_t parent_pagenum, pagenum_t left_pagenum, pagenum_t right_pagenum); 
int insert_into_new_root(page_t * left, int64_t key, page_t * right, pagenum_t left_pagenum, pagenum_t right_pagenum);
int insert_into_internal(internal_page_t * internal, int64_t key, pagenum_t pagenum, pagenum_t parent_pagenum);
int insert_into_internal_after_splitting(internal_page_t * internal, int64_t key, pagenum_t record_pagenum, pagenum_t parent_pagenum, pagenum_t this_pagenum); 

// Deletion.
int delete(int64_t key); 
int delete_entry_leaf(pagenum_t leaf_pagenum, int64_t key); 
int remove_entry_from_leaf(pagenum_t leaf_pagenum, int64_t key); 
int adjust_root_leaf(pagenum_t root_pagenum); 
int delete_entry_internal(pagenum_t this_pagenum, int64_t key);
int remove_entry_from_internal(pagenum_t this_pagenum, int64_t key);
int64_t find_parent_key(pagenum_t this_pagenum, int64_t key);
