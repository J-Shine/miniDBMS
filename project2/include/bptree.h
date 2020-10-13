// bptree.h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include<string.h>
#include "file.h"

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
	char reserved[104];
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


leaf_page_t * find_leaf(int64_t key);
int find(int64_t key, char * ret);



