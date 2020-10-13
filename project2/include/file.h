#define _GNU_SOURCE
#include<stdint.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>

typedef uint64_t pagenum_t;
/*

struct page_t{
	// in-memory page structure
	struct page_t * free; // [0-7]
	struct page_t * parent; // [0-7]
	int is_leaf; // [8-11]
	int num_keys; // [12-15]
	void ** pointers;
	uint64_t * keys; // [128-4095]
	struct page_t * next; // [120-127] Used for queue.
};
typedef struct page_t page_t;
*/

struct page_t{
	pagenum_t buffer[512];
};

typedef struct page_t page_t;

extern int fd;
extern page_t free_pages; 


// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page();

// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page
void file_write_page(pagenum_t pagenum, const page_t* src);


