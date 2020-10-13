#include "file.h"

int fd;
page_t free_pages;

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(){
	pagenum_t cur_free_num;
	pagenum_t next_free_num;
	pread(fd, &next_free_num, 8, 0);
	cur_free_num = next_free_num;
	pread(fd, &next_free_num, 8, cur_free_num * 4096);
	pwrite(fd, &next_free_num, 8, 0);
	
	return cur_free_num;	
}

// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum){
	pagenum_t cur_free_num = 0;
	pagenum_t next_free_num;
	pread(fd, &next_free_num, 8, 0);
	while(next_free_num != 0){
		cur_free_num = next_free_num;
		pread(fd, &next_free_num, 8, cur_free_num * 4096);
	}
	next_free_num = pagenum;
	pwrite(fd, &next_free_num, 8, cur_free_num * 4096); 
	printf("next_free_num from header: %lu\n", next_free_num);
	uint64_t zero = 0;
	pwrite(fd, &zero, 8, next_free_num * 4096); 
}

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(pagenum_t pagenum, page_t* dest){
	pread(fd, dest->buffer, 4096, pagenum * 4096); 
}

// Write an in-memory page(src) to the on-disk page
void file_write_page(pagenum_t pagenum, const page_t* src){
	pwrite(fd, src->buffer, 4096, pagenum *4096);
}

