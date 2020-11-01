#include "file.h"
#include <stdio.h>
#include <string.h>

int unique_id[11];
char * unique_path[21];
int unique_id_cnt = 0;

page_t free_pages;

void copy_page_t(page_t * des, page_t * src){
	for(int i = 0; i < 512; i++){
		des->buffer[i] = src->buffer[i];
	}
}

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */

// set header if doesn't exist
void set_header(int file_descriptor){
	page_t * c = (page_t *)malloc(sizeof(page_t));
	file_read_page(file_descriptor, 0, c);
	c->buffer[0] = 0;
	c->buffer[1] = 0;
	c->buffer[2] = 1;
	file_write_page(file_descriptor, 0, c);

	free(c);
}

// set free pages
void set_free_pages(int file_descriptor){
	page_t * header = (page_t *)malloc(sizeof(page_t));
	page_t * free_page1 = (page_t *)malloc(sizeof(page_t));
	//printf("inside set_free_pagess\n");
	
	// get current free number
	file_read_page(file_descriptor, 0, header);

	if(header->buffer[0] == 0)
		header->buffer[0] = 1;
	int cnt = 1;
	int64_t next_free_num = header->buffer[0];
	while(cnt < 5 && next_free_num != 0){
		file_read_page(file_descriptor, next_free_num, free_page1);
		next_free_num = free_page1->buffer[0];
		//printf("next_free_numm %ld \n", next_free_num);
		cnt++;
	}
	while(cnt < 5){
		printf("xxx\n");
		file_free_page(file_descriptor, header->buffer[2]);
		header->buffer[2] = header->buffer[2] + 1;
		cnt++;
	}
	file_write_page(file_descriptor, 0, header);
}

int file_open(char * pathname){
	int fd;
	int i = 1;
	while(unique_id[i] != -1 && i <= 10){
		// 이미 unique_id가 존재하는 파일인 경우 fd를 unique_id에 다시 할당해주고 unique_id를 리턴
		if(strcmp(pathname, unique_path[i]) == 0){
			if((fd = open(pathname, O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO)) == -1){
				perror("open");
				return -1;
			}
			unique_id[i] = fd;
			return i;
		}
		i++;
		//printf("i is %d\n", i);
	}
	if(unique_id_cnt >= 10){
		printf("The unique table is full. No more new id is available.\n");
		return -1;
	}
	if((fd = open(pathname, O_EXCL|O_CREAT|O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO)) == -1){
		if((fd = open(pathname, O_CREAT|O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO)) == -1){
			perror("open");
			return -1;
		}
		unique_id[i] = fd;
		unique_path[i] = (char *)malloc(sizeof(char) * 21);
		unique_path[i] = strncpy(unique_path[i], pathname, sizeof(char) * 21);
		unique_id_cnt++;
		return i;
	}
	// read header
	page_t * header_c = (page_t *)malloc(sizeof(page_t));
	file_read_page(fd, 0, header_c);

	// set header and free pages
	//printf("fd: %d\n", fd);
	//printf("header_c->buffer[0]: %ld\n", header_c->buffer[0]);
	set_header(fd);
	set_free_pages(fd);
	free(header_c);

	unique_id[i] = fd;
	unique_path[i] = (char *)malloc(sizeof(char) * 21);
	unique_path[i] = strncpy(unique_path[i], pathname, sizeof(char) * 21);
	unique_id_cnt++;

	return i;
}

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int file_descriptor){
	page_t * header = (page_t *)malloc(sizeof(page_t));
	page_t * free_page1 = (page_t *)malloc(sizeof(page_t));

	// get current free number
	file_read_page(file_descriptor, 0, header);
	pagenum_t cur_free_num = header->buffer[0];

	// get next free number
	file_read_page(file_descriptor, cur_free_num, free_page1);
	header->buffer[0] = free_page1->buffer[0];
	
	// write to disk
	file_write_page(file_descriptor, 0, header); 

	free(header);
	
	return cur_free_num;	
}

// Free an on-disk page to the free page list
void file_free_page(int file_descriptor, pagenum_t pagenum){

	//printf("freeing page %ld\n", pagenum);
	// set new free page number to end of the list
	page_t * cur_free_num_page = (page_t *)malloc(sizeof(page_t));
	page_t * next_free_num_page = (page_t *)malloc(sizeof(page_t));

	pread(file_descriptor, next_free_num_page, 8, 0);
	while(next_free_num_page->buffer[0] != 0){
		cur_free_num_page->buffer[0] = next_free_num_page->buffer[0];
		pread(file_descriptor, next_free_num_page, 8, (cur_free_num_page->buffer[0]) * 4096);
	}
	//printf("next_free_num1 %ld\n", next_free_num_page->buffer[0]);
	next_free_num_page->buffer[0] = pagenum;
	//printf("cur_free_num %ld\n", cur_free_num_page->buffer[0]);
	//printf("file_descriptor: %d\n", file_descriptor);
	//printf("cur_free_num: %ld\n", cur_free_num_page->buffer[0]);
	pwrite(file_descriptor, next_free_num_page, 8, (cur_free_num_page->buffer[0]) * 4096); 

	// make page clean
	page_t * empty_page = (page_t *)malloc(sizeof(page_t));
	for(int i = 1; i < 512; i++){
		empty_page->buffer[i] = 0;
	}
	pwrite(file_descriptor, empty_page, 4096, (next_free_num_page->buffer[0]) * 4096); 
}

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int file_descriptor, pagenum_t pagenum, page_t* dest){
	page_t * c = (page_t *)malloc(sizeof(page_t));
	pread(file_descriptor, c, 4096, pagenum * 4096); 
	for(int i = 0; i < 512; i++){
		dest->buffer[i] = c->buffer[i];
	}
	free(c);
}

// Write an in-memory page(src) to the on-disk page
void file_write_page(int file_descriptor, pagenum_t pagenum, const page_t* src){
	page_t * c = (page_t *)malloc(sizeof(page_t));
	for(int i = 0; i < 512; i++){
		c->buffer[i] = src->buffer[i];
	}
	pwrite(file_descriptor, c, 4096, pagenum *4096);
	free(c);
}


