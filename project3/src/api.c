// api.c

#include "api.h"

buffer_t ** buffer_pool;
int num_buffer;

int init_db(int num_buf){

	// 전역변수인 num_buffer에 입력받은 num_buf를 저장
	num_buffer = num_buf;

	// buffer 배열 동적 할당 후 초기화
	buffer_pool = (buffer_t **)malloc(sizeof(buffer_t) * num_buf);
	for(int i = 0; i < num_buf; i++){
		buffer_pool[i] = (buffer_t *)malloc(sizeof(buffer_t));
		buffer_pool[i]->table_id = -1;
		buffer_pool[i]->page_num = -1;
		buffer_pool[i]->is_dirty = -1;
		buffer_pool[i]->is_pinned = -1;
		buffer_pool[i]->LRU_next = NULL;
	}

	// table 배열을 초기화
	for(int i = 1; i < 11; i++){
		unique_id[i] = -1;
	}

	return 0;

}


int open_table(char * pathname){

	int unique_idx = file_open(pathname);

	// read header
	page_t * header_c = (page_t *)malloc(sizeof(page_t));
	file_read_page(0, header_c);
	header_page_t * header = (header_page_t *)header_c;

	// set header and free pages
	if(header->free_start == 0){
		set_header();
		set_free_pages();
	}
	free(header_c);

	return unique_id[unique_idx];
}

int db_insert(int table_id, int64_t key, char * value){
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

int db_find(int table_id, int64_t key, char * ret_val){
	if(unique_id[table_id] <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}
	if((find(unique_id[table_id], key, ret_val)) < 0){
		return -1;
	}
	return 0;
}

int db_delete(int table_id, int64_t key){
	if(fd <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}
	int rtn;
	rtn = delete(key);
	return rtn;
}

int close_table(int table_id){

}

int shutdown_db(void){
	for(int i = 0; i < num_buffer; i++){
		free(buffer_pool[i]);
	}
	free(buffer_pool);
	
}
