// api.c
#include "api.h"
#include <trx.h>

int init_db(int num_buf){

	// 전역변수인 num_buffer에 입력받은 num_buf를 저장
	num_buffer = num_buf;

	// buffer 배열 동적 할당 후 초기화
	buffer_pool = (buffer_t **)malloc(sizeof(buffer_t) * num_buf);
    buffer_manager_latch = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(buffer_manager_latch, 0);
    //buffer_manager_cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    //pthread_cond_init(buffer_manager_cond, 0);
	for(int i = 0; i < num_buf; i++){
		buffer_pool[i] = (buffer_t *)malloc(sizeof(buffer_t));
		buffer_pool[i]->table_id = -1;
		buffer_pool[i]->page_num = -1;
		buffer_pool[i]->is_dirty = -1;
        buffer_pool[i]->page_latch = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(buffer_pool[i]->page_latch, 0);
        //buffer_pool[i]->page_cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
        //pthread_cond_init(buffer_pool[i]->page_cond, 0);
		buffer_pool[i]->LRU_prev = (buffer_t *)malloc(sizeof(buffer_t));
		buffer_pool[i]->LRU_next = (buffer_t *)malloc(sizeof(buffer_t));

	}

	// table 배열을 초기화
	for(int i = 1; i < 11; i++){
		unique_id[i] = -1;
	}

	// LRU head와 tail 동적할당
	LRU_head = NULL;
	LRU_tail = (buffer_t *)malloc(sizeof(buffer_t));

    /* initialize lock table */
    init_lock_table();

	return 0;
}


int open_table(char * pathname){

	int unique_idx = file_open(pathname);

	return unique_id[unique_idx];
}

int db_insert(int table_id, int64_t key, char * value, int trx_id){
	if(unique_id[table_id] <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}

	// call insert
	if((insert(table_id, key, value, trx_id, 0)) < 0)
		return -1;
	return 0;

}

int trx_begin(){
    return trx_make_id();
}

int trx_commit(int trx_id){
    return trx_clean(trx_id, 0);
}

int db_find(int table_id, int64_t key, char * ret_val, int trx_id){
	if(unique_id[table_id] <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}
    /* find the record and acquire lock */
    int result = find(table_id, key, ret_val, trx_id, 0);

    /* abort thread if find operation fail or deadlock detected */
    if(result < 0){
        trx_clean(trx_id, -1);
    }
	return result;
}

int db_update(int table_id, int64_t key, char* values, int trx_id){
	if(unique_id[table_id] <= 2){
		printf("파일이 열려있지 않습니다.\n");
		return -1;
	}
    /* find the record, acquire lock and update the value */
    int result = find(table_id, key, values, trx_id, 1);

    /* abort thread if find operation fail or deadlock detected */
    if(result < 0){
        trx_clean(trx_id, -1);
    }
	return result;
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
