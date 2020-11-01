// api.c

#include "api.h"


int init_db(int num_buf){

	// 전역변수인 num_buffer에 입력받은 num_buf를 저장
	num_buffer = num_buf;

	// buffer 배열 동적 할당 후 초기화
	buffer_pool = (buffer_t **)malloc(sizeof(buffer_t) * num_buf);
	for(int i = 0; i < num_buf; i++){
		buffer_pool[i] = (buffer_t *)malloc(sizeof(buffer_t));
		//printf("sizeof(buffer_t) is %ld\n", sizeof(buffer_t));
		buffer_pool[i]->table_id = -1;
		buffer_pool[i]->page_num = -1;
		buffer_pool[i]->is_dirty = -1;
		buffer_pool[i]->is_pinned = -1;
		buffer_pool[i]->LRU_prev = (buffer_t *)malloc(sizeof(buffer_t));
		buffer_pool[i]->LRU_next = (buffer_t *)malloc(sizeof(buffer_t));

	}

	// table 배열을 초기화
	for(int i = 1; i < 11; i++){
		unique_id[i] = -1;
	}

	// LRU head와 tail 동적할당
	LRU_head = NULL;
	/*
	LRU_head = (buffer_t *)malloc(sizeof(buffer_t));
	LRU_head->LRU_prev = (buffer_t *)malloc(sizeof(buffer_t));
	LRU_head->LRU_next = (buffer_t *)malloc(sizeof(buffer_t));
	*/
	LRU_tail = (buffer_t *)malloc(sizeof(buffer_t));
	/*
	LRU_tail->LRU_prev = (buffer_t *)malloc(sizeof(buffer_t));
	LRU_tail->LRU_next = (buffer_t *)malloc(sizeof(buffer_t));
	*/

	return 0;
}


int open_table(char * pathname){

	int unique_idx = file_open(pathname);

	return unique_id[unique_idx];
}

int db_insert(int table_id, int64_t key, char * value){
	if(unique_id[table_id] <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}

	// call insert
	if((insert(table_id, key, value)) < 0)
		return -1;
	return 0;

}


int db_find(int table_id, int64_t key, char * ret_val){
	if(unique_id[table_id] <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}
	if((find(table_id, key, ret_val)) < 0){
		return -1;
	}
	return 0;
}
/*

int db_delete(int table_id, int64_t key){
	if(fd <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}
	int rtn;
	rtn = delete(key);
	return rtn;
}
*/

int close_table(int table_id){
	for(int i = 0; i < num_buffer; i++){
		if(buffer_pool[i]->table_id == table_id){
			printf("x\n");
			buffer_write_disk(i);
		}
	}
}

int shutdown_db(void){
	for(int i = 0; i < num_buffer; i++){
		free(buffer_pool[i]);
	}
	free(buffer_pool);
	
}
