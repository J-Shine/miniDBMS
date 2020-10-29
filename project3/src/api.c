// api.c

#include "api.h"

int open_table(char * pathname){

	fd = file_open(pathname);

	page_t * header_c = (page_t *)malloc(sizeof(page_t));
	file_read_page(0, header_c);
	header_page_t * header = (header_page_t *)header_c;
	if(header->free_start == 0){
		set_header();
		set_free_pages();
	}
	free(header_c);

	return fd;
}

int db_insert(int64_t key, char * value){
	if(fd <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}
	// find header
	page_t * c = (page_t *)malloc(sizeof(page_t));
	file_read_page(0, c);
	header_page_t * header = (header_page_t *)c;
	if(header->num_pages == 0)
		set_header();

	// call insert
	if((insert(key, value)) < 0)
		free(header);
		return -1;
	free(header);
	return 0;

}

int db_find(int64_t key, char * ret_val){
	if(fd <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}
	if((find(key, ret_val)) < 0){
		return -1;
	}
	return 0;
}

int db_delete(int64_t key){
	if(fd <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}
	int rtn;
	rtn = delete(key);
	return rtn;
}
