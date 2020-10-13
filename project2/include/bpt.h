#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "file.h"
#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

// Default order is 4.
#define LEAF_ORDER 32 
#define INTERNAL_ORDER 249 

// Constants for printing part or all of the GPL license.
#define LICENSE_FILE "LICENSE.txt"
#define LICENSE_WARRANTEE 0
#define LICENSE_WARRANTEE_START 592
#define LICENSE_WARRANTEE_END 624
#define LICENSE_CONDITIONS 1
#define LICENSE_CONDITIONS_START 70
#define LICENSE_CONDITIONS_END 625

// TYPES.

/* Type representing the record
 * to which a given key refers.
 * In a real B+ tree system, the
 * record would hold data (in a database)
 * or a file (in an operating system)
 * or some other information.
 * Users can rewrite this part of the code
 * to change the type and content
 * of the value field.
 */

struct key_value_t{
	int64_t key; // 8byte
	char * value[120]; // 120byte
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

/* Type representing a page_t in the B+ tree.
 * This type is general enough to serve for both
 * the leaf and the internal page_t.
 * The heart of the page_t is the array
 * of keys and the array of corresponding
 * pointers.  The relation between keys
 * and pointers differs between leaves and
 * internal page_ts.  In a leaf, the index
 * of each key equals the index of its corresponding
 * pointer, with a maximum of order - 1 key-pointer
 * pairs.  The last pointer points to the
 * leaf to the right (or NULL in the case
 * of the rightmost leaf).
 * In an internal page_t, the first pointer
 * refers to lower page_ts with keys less than
 * the smallest key in the keys array.  Then,
 * with indices i starting at 0, the pointer
 * at i + 1 points to the subtree with keys
 * greater than or equal to the key in this
 * page_t at index i.
 * The num_keys field is used to keep
 * track of the number of valid keys.
 * In an internal page_t, the number of valid
 * pointers is always num_keys + 1.
 * In a leaf, the number of valid pointers
 * to data is always num_keys.  The
 * last leaf pointer points to the next leaf.
 */
typedef struct page_t {
    void ** pointers; // pointer의 배열
    int * keys; // key의 배열
    struct page_t * parent;
    bool is_leaf;
    int num_keys;
    struct page_t * next; // Used for queue.
} page_t;

// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * page_t.  Every page_t has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal page_t has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
extern int order;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
extern page_t * queue;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
//extern bool verbose_output;


// FUNCTION PROTOTYPES.

// Output and utility.

void license_notice( void );
void print_license( int licence_part );
void usage_1( void );
void usage_2( void );
void usage_3( void );
void enqueue( page_t * new_page_t );
page_t * dequeue( void );
int height( page_t * root );
int path_to_root( page_t * root, page_t * child );
void print_leaves( page_t * root );
void print_tree( page_t * root );
void find_and_print(page_t * root, int64_t key, bool verbose); 
leaf_page_t * find_leaf(int64_t key);
char * find(int64_t key);

int cut( int length );

// Insertion.
key_value_t * make_record(char * value);
page_t * make_page_t( void );
page_t * make_leaf( void );
int get_left_index(page_t * parent, page_t * left);
page_t * insert_into_leaf( page_t * leaf, int64_t key, key_value_t * pointer );
page_t * insert_into_leaf_after_splitting(page_t * root, page_t * leaf, int64_t key,
                                        key_value_t * pointer);
page_t * insert_into_page_t(page_t * root, page_t * parent, 
        int left_index, int64_t key, page_t * right);
page_t * insert_into_page_t_after_splitting(page_t * root, page_t * parent,
                                        int left_index,
        int64_t key, page_t * right);
page_t * insert_into_parent(page_t * root, page_t * left, int64_t key, page_t * right);
page_t * insert_into_new_root(page_t * left, int64_t key, page_t * right);
page_t * start_new_tree(int64_t key, key_value_t * pointer);
page_t * insert( page_t * root, int64_t key, char * value );

// Deletion.

int get_neighbor_index( page_t * n );
page_t * adjust_root(page_t * root);
page_t * coalesce_page_ts(page_t * root, page_t * n, page_t * neighbor,
                      int neighbor_index, int k_prime);
page_t * redistribute_page_ts(page_t * root, page_t * n, page_t * neighbor,
                          int neighbor_index,
        int k_prime_index, int k_prime);
page_t * delete_entry( page_t * root, page_t * n, int64_t key, void * pointer );
page_t * delete( page_t * root, int64_t key );

void destroy_tree_page_ts(page_t * root);
page_t * destroy_tree(page_t * root);

#endif /* __BPT_H__*/
