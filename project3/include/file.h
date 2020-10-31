#define _GNU_SOURCE
#include<stdint.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>

extern int unique_id[11];
extern char * unique_path[21];
extern int unique_id_cnt;

typedef uint64_t pagenum_t;

struct page_t{
	pagenum_t buffer[512];
};

typedef struct page_t page_t;

extern int fd;
extern page_t free_pages; 

// open file
int file_open(char * pathname);

// copy page from src to des
void copy_page_t(page_t * des, page_t * src);

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page();

// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page
void file_write_page(pagenum_t pagenum, const page_t* src);


