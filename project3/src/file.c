#include "file.h"
#include <stdio.h>
#include <string.h>

int unique_id[11];
char * unique_path[21];
int unique_id_cnt = 0;

int fd;
page_t free_pages;

void copy_page_t(page_t * des, page_t * src){
	for(int i = 0; i < 512; i++){
		des->buffer[i] = src->buffer[i];
	}
}

int file_open(char * pathname){
	int i = 1;
	while(unique_id[i] != -1 && i <= 10){
		// 이미 unique_id가 존재하는 파일인 경우 fd를 unique_id에 다시 할당해주고 unique_id를 리턴
		if(strcmp(pathname, unique_path[i]) == 0){
			if((fd = open(pathname, O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO)) == -1){
				/*
				printf("pathname: %s\n", pathname);
				printf("error1\n");
				*/
				perror("open");
				return -1;
			}
			/*
			printf("pathname: %s\n", pathname);
			printf("unique_id[%d]: %d\n", i, unique_id[i]);
			printf("unique_id_cnt: %d\n", unique_id_cnt);
			*/
			unique_id[i] = fd;
			return i;
		}
		i++;
		printf("i is %d\n", i);
	}
	if(unique_id_cnt >= 10){
		printf("The unique table is full. No more new id is available.\n");
		return -1;
	}
	if((fd = open(pathname, O_EXCL|O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO)) == -1){
		if((fd = open(pathname, O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO)) == -1){
			/*
			printf("pathname: %s\n", pathname);
			printf("error2\n");
			*/
			perror("open");
			return -1;
		}
		unique_id[i] = fd;
		unique_path[i] = (char *)malloc(sizeof(char) * 21);
		unique_path[i] = strncpy(unique_path[i], pathname, sizeof(char) * 21);
		unique_id_cnt++;
		/*
		printf("pathname: %s\n", pathname);
		printf("unique_id[%d]: %d\n", i, unique_id[i]);
		printf("unique_id_cnt: %d\n", unique_id_cnt);
		*/
		return i;
	}
	unique_id[i] = fd;
	unique_path[i] = (char *)malloc(sizeof(char) * 21);
	unique_path[i] = strncpy(unique_path[i], pathname, sizeof(char) * 21);
	unique_id_cnt++;
	/*
	printf("pathname: %s\n", pathname); 
	printf("unique_id[%d]: %d\n", i, unique_id[i]); 
	printf("unique_id_cnt: %d\n", unique_id_cnt);
	*/
	return i;
}

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(){
	page_t * header = (page_t *)malloc(sizeof(page_t));
	page_t * free_page1 = (page_t *)malloc(sizeof(page_t));

	// get current free number
	file_read_page(0, header);
	pagenum_t cur_free_num = header->buffer[0];
	//printf("file_alloc_page: cur_free_num1: %ld\n", cur_free_num);

	// get next free number
	file_read_page(cur_free_num, free_page1);
	header->buffer[0] = free_page1->buffer[0];
	/*
	printf("file_alloc_page: free_page1->buffer[0]: %ld\n", free_page1->buffer[0]);
	printf("file_alloc_page: header->buffer[0]: %ld\n", header->buffer[0]);
	*/
	
	// write to disk
	file_write_page(0, header); 
	/*
	printf("file_alloc_page: header->buffer[0]: %ld\n", header->buffer[0]);
	printf("file_alloc_page: header->buffer[0]: %ld\n", header->buffer[1]);
	printf("file_alloc_page: header->buffer[0]: %ld\n", header->buffer[2]);
	*/

	free(header);
	
	return cur_free_num;	
}

// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum){

	printf("freeing page %ld\n", pagenum);
	// set new free page number to end of the list
	pagenum_t * cur_free_num = (pagenum_t *)malloc(sizeof(pagenum_t));
	pagenum_t * next_free_num = (pagenum_t *)malloc(sizeof(pagenum_t));;
	pread(fd, next_free_num, 8, 0);
	while(*next_free_num != 0){
		*cur_free_num = *next_free_num;
		pread(fd, next_free_num, 8, (*cur_free_num) * 4096);
	}
	//printf("*next_free_num1 %ld\n", *next_free_num);
	*next_free_num = pagenum;
	//printf("*next_free_num2 %ld\n", *next_free_num);
	//printf("*cur_free_num %ld\n", *cur_free_num);
	pwrite(fd, next_free_num, 8, (*cur_free_num) * 4096); 

	// make page clean
	page_t * empty_page = (page_t *)malloc(sizeof(page_t));
	for(int i = 1; i < 512; i++){
		empty_page->buffer[i] = 0;
	}
	pwrite(fd, empty_page, 4096, (*next_free_num) * 4096); 
}

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, page_t* dest){
	pread(fd, dest->buffer, 4096, pagenum * 4096); 
}

// Write an in-memory page(src) to the on-disk page
void file_write_page(pagenum_t pagenum, const page_t* src){
	pwrite(fd, src->buffer, 4096, pagenum *4096);
}

