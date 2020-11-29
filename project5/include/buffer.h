#include "file.h"
#include <pthread.h>

struct buffer_t{
	page_t page; // 4096byte
	int table_id;
	int64_t page_num;
	int is_dirty;
    pthread_mutex_t * page_latch;
    //pthread_cond_t * page_cond;
	struct buffer_t * LRU_prev;
	struct buffer_t * LRU_next;
	int64_t key; // 8byte
	char value[120]; // 120byte
};	
typedef struct buffer_t buffer_t;

extern buffer_t ** buffer_pool;
extern pthread_mutex_t * buffer_manager_latch;
//extern pthread_cond_t * buffer_manager_cond;
extern int num_buffer;

extern buffer_t * LRU_head;
extern buffer_t * LRU_tail;

void print_buffer();
void print_LRU_list();

int pin(int table_id, pagenum_t pagenum);
int unpin(int table_id, pagenum_t pagenum);

int LRU_push_tail(int idx);
int LRU_pop_head(int * rtn_table_id, int * rtn_pagenmum, int * rtn_idx);

int buffer_read_buffer(int table_id, pagenum_t pagenum, page_t * dest);

int buffer_check_buffer(int table_id, pagenum_t pagenum, page_t * dest);

int buffer_read_disk(int table_id, pagenum_t pagenum, page_t * dest, int check);

int buffer_write_buffer(int table_id, pagenum_t pagenum, page_t * src);

int buffer_write_disk(int idx);

pagenum_t file_alloc_page_buffer(int table_id);
void set_free_pages_buffer(int table_id);
