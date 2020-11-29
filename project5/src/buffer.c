#include "buffer.h"
   
buffer_t ** buffer_pool;
pthread_mutex_t * buffer_manager_latch;
//pthread_cond_t * buffer_manager_cond;
int num_buffer;

buffer_t * LRU_head;
buffer_t * LRU_tail;

void print_buffer(){
	printf("print all buffer pool\n");
	for(int i = 0; i < num_buffer; i++){
		for(int j = 0; j < 10; j++){
			printf("buffer_pool[%d]->page.buffer[%d] %ld\n", i, j, buffer_pool[i]->page.buffer[j]);
		}
		printf("buffer_pool[%d]->table_id: %d\n", i, buffer_pool[i]->table_id);
		printf("buffer_pool[%d]->page_num: %ld\n", i, buffer_pool[i]->page_num);
		printf("buffer_pool[%d]->is_dirty: %d\n", i, buffer_pool[i]->is_dirty);
	}
}

void print_LRU_list(){
	printf("print all LRU list\n");
	if(LRU_head == NULL && LRU_tail == NULL) return;
	pagenum_t next_table_id = LRU_head->table_id;
	pagenum_t next_page_num = LRU_head->page_num;
	buffer_t * next_LRU = (buffer_t *)malloc(sizeof(buffer_t));
	next_LRU = LRU_head;
	while(next_LRU != NULL){
		printf("LRU_head->table_id %d\n", next_LRU->table_id);
		printf("LRU_head->page_num %ld\n", next_LRU->page_num);
		next_LRU = next_LRU->LRU_next;
		if(next_LRU != NULL){
			next_table_id = next_LRU->table_id;
			next_page_num = next_LRU->page_num;
		}
		else
			return;
	}
}

int buffer_read_buffer(int table_id, pagenum_t pagenum, page_t * dest){
    printf("start buffer_read_buffer()\n");
	int check = buffer_check_buffer(table_id, pagenum, dest);
    printf("?\n");
	if(check == -1){
        printf("end buffer_read_buffer()1\n");
		return 0;
    }
	if(check > -1){
        printf("end buffer_read_buffer()2.5\n");
		buffer_read_disk(table_id, pagenum, dest, check);
        printf("end buffer_read_buffer()2\n");
		return 0;
	}
    printf("end buffer_read_buffer()3\n");
    return -1;
}

int buffer_check_buffer(int table_id, pagenum_t pagenum, page_t * dest){
    printf("start buffer_check_buffer()\n");
	int i = 0;
	while(i < num_buffer && buffer_pool[i]->table_id != -1){
		// buffer_pool에 page가 존재하는 경우 dest에 넣어서 돌려주고 LRU list에 추가
		if(buffer_pool[i]->table_id == table_id && buffer_pool[i]->page_num == pagenum){
			for(int j = 0; j < 512; j++){
				dest->buffer[j] = buffer_pool[i]->page.buffer[j];
			}
			pin(table_id, pagenum);
			LRU_push_tail(i);
            printf("end buffer_check_buffer()1\n");
			return -1;
		}
		i++;
	}
    printf("end buffer_check_buffer()2\n");
	return i;
}

int LRU_push_tail(int idx){
	// LRU list가 비어있고 첫 원소인 경우
	if(LRU_head == NULL){
		LRU_head = buffer_pool[idx];
		LRU_head->LRU_next = NULL;
		LRU_head->LRU_prev = NULL;
		LRU_tail = LRU_head;
		return 0;
	}

	// 이미 중간에 붙어있을 때
	if(buffer_pool[idx]->LRU_prev != NULL && buffer_pool[idx]->LRU_next != NULL){
		/*
		printf("inside LRU_push_tail middle\n");
		printf("buffer_pool[idx]->page_num %ld\n", buffer_pool[idx]->page_num);
		*/
		buffer_pool[idx]->LRU_prev->LRU_next = buffer_pool[idx]->LRU_next;
		buffer_pool[idx]->LRU_next->LRU_prev = buffer_pool[idx]->LRU_prev;
	}
	// tail일 때
	else if(buffer_pool[idx] == LRU_tail){
		/*
		printf("inside LRU_push_tail tail\n");
		printf("buffer_pool[idx]->page_num %ld\n", buffer_pool[idx]->page_num);
		*/
		return 0;
	}
	// head일 때
	else if(buffer_pool[idx] == LRU_head){
		/*
		printf("inside LRU_push_tail head\n");
		printf("buffer_pool[idx]->page_num %ld\n", buffer_pool[idx]->page_num);
		*/
		LRU_head = LRU_head->LRU_next;
		LRU_head->LRU_prev = NULL;
	}

	buffer_pool[idx]->LRU_prev = LRU_tail;
	buffer_pool[idx]->LRU_next = NULL;
	LRU_tail->LRU_next = buffer_pool[idx];
	LRU_tail = buffer_pool[idx];

	return 0;
}

int LRU_pop_head(int * rtn_table_id, int * rtn_pagenum, int * rtn_idx){
	//printf("inside pop head\n");

	int is_dirty = -1;
	printf("LRU_head->table_id %d\n", LRU_head->table_id);
	printf("LRU_head->page_num%ld\n", LRU_head->page_num);
	printf("LRU_head->is_dirty%d\n", LRU_head->is_dirty);
	is_dirty = LRU_head->is_dirty;
	printf("LRU_head pagenum %ld\n", LRU_head->page_num);
			for(int i = 0; i < 10; i++){
				printf("before kicked to disk buffer_pool[%d]->page2 %ld\n", i, LRU_head->page.buffer[i]);
			}
	*rtn_table_id = LRU_head->table_id;
	*rtn_pagenum = LRU_head->page_num;

	int i = 0;
	//printf("num_buffer %d\n", num_buffer);
	while(i < num_buffer && buffer_pool[i]->table_id != -1){
		//printf("buffer_pool[i]->table_id %d\n", buffer_pool[i]->table_id);
		if(buffer_pool[i]->table_id == LRU_head->table_id && buffer_pool[i]->page_num == LRU_head->page_num){
			*rtn_idx = i;
			break;
		}
		i++;
	}
	LRU_head = LRU_head->LRU_next;
	return is_dirty;
}

int pin(int table_id, pagenum_t pagenum){
    printf("start pin()\n");
    pthread_mutex_lock(buffer_manager_latch);
	int idx = 0;
	while(idx < num_buffer && buffer_pool[idx]->table_id != -1){
		if(buffer_pool[idx]->table_id == table_id && buffer_pool[idx]->page_num == pagenum){
            pthread_mutex_lock(buffer_pool[idx]->page_latch);
            pthread_mutex_unlock(buffer_manager_latch);
            printf("end pin()\n");
            return 0;
            /*
			if(buffer_pool[idx]->is_pinned > 0){
				printf("This page is pinned. Can't read this page.\n");
				return -1;
			}
			else if(buffer_pool[idx]->is_pinned == 0 || buffer_pool[idx]->is_pinned == -1){
				buffer_pool[idx]->is_pinned = 1;
				return 0;
			}
			else{
				printf("pin error\n");
				return -1;
			}
            */
		}
		idx++;
	}
    printf("pin error\n");
    return -1;
}

int unpin(int table_id, pagenum_t pagenum){
    printf("start unpin()\n");
	int i = 0;
	while(i < num_buffer && buffer_pool[i]->table_id != -1){
		if(buffer_pool[i]->table_id == table_id && buffer_pool[i]->page_num == pagenum){
            pthread_mutex_unlock(buffer_pool[i]->page_latch);
            printf("end unpin()\n");
            return 0;
            /*
			if(buffer_pool[i]->is_pinned >= 1){
				buffer_pool[i]->is_pinned--;
				return 0;
			}
			else{
				printf("unpin error\n");
				return -1;
			}
            */
		}
		i++;
	}
    printf("unpin error\n");
    return -1;
}


int buffer_read_disk(int table_id, pagenum_t pagenum, page_t * dest, int idx){
    printf("start buffer_read_disk()\n");
	// buffer에서 못 찾았고 buffer에 자리가 있는 경우 disk에서 가져와서 buffer에 쓰고 dest에 넣어서 돌려줌
	// LRU list에도 추가
	if(idx < num_buffer){
		printf("here1\n");
		file_read_page(unique_id[table_id], pagenum, &(buffer_pool[idx]->page)); 
		/*
		for(int i = 0; i < 10; i++){
			printf("buffer_pool[idx]->page.buffer[i] in buffer_read_disk() %ld\n", buffer_pool[idx]->page.buffer[i]);
		}
		*/
		buffer_pool[idx]->table_id = table_id;
		buffer_pool[idx]->page_num = pagenum;
		buffer_pool[idx]->is_dirty = 0;
		buffer_pool[idx]->LRU_prev = NULL;
		buffer_pool[idx]->LRU_next = NULL;
		pin(table_id, pagenum);
		LRU_push_tail(idx);
		for(int i = 0; i < 512; i++){
			dest->buffer[i] = buffer_pool[idx]->page.buffer[i];
		}
        printf("end buffer_read_disk()1\n");
		return 0;
	}
	// buffer에서 못 찾았고 buffer에 자리가 없는 경우 LRU에서 head 꺼내 disk에 write 후 그 자리 buffer에
	// disk에서 가져와서 쓴 후 dest에 넣어서 돌려주고 LRU list에 추가
	else if(idx ==num_buffer){
		// head에서 꺼내서 disk에 write
		int w_table_id = 0;
		int w_pagenum = 0;
		int w_idx = 0;
		int is_dirty = LRU_pop_head(&w_table_id, &w_pagenum, &w_idx);
		if(is_dirty == 0){
			printf("inside is_dirty == 0\n");
			file_read_page(unique_id[table_id], pagenum, &(buffer_pool[w_idx]->page));
			buffer_pool[w_idx]->table_id = table_id;
			buffer_pool[w_idx]->page_num = pagenum;
			buffer_pool[w_idx]->is_dirty = 0;
			buffer_pool[w_idx]->LRU_prev = NULL;
			buffer_pool[w_idx]->LRU_next = NULL;
			pin(table_id, pagenum);
			LRU_push_tail(w_idx);
			for(int j = 0; j < 512; j++){
				dest->buffer[j] = buffer_pool[w_idx]->page.buffer[j];
			}
            printf("end buffer_read_disk()2\n");
			return 0;
		}
		else if(is_dirty == 1){
			for(int i = 0; i < 10; i++){
				printf("before kicked to disk buffer_pool[%d]->page1 %ld\n", w_idx, buffer_pool[w_idx]->page.buffer[i]);
			}
			buffer_write_disk(w_idx);

			// read new page to buffer
			file_read_page(unique_id[table_id], pagenum, &(buffer_pool[w_idx]->page));
			buffer_pool[w_idx]->table_id = table_id;
			buffer_pool[w_idx]->page_num = pagenum;
			buffer_pool[w_idx]->is_dirty = 0;
			pin(table_id, pagenum);
			LRU_push_tail(w_idx);
			for(int j = 0; j < 512; j++){
				dest->buffer[j] = buffer_pool[w_idx]->page.buffer[j];
			}
            printf("end buffer_read_disk()3\n");
			return 0;
		}
	}
	else{
		printf("buffer_read_disk() error\n");
		return -1;
	}
}

int buffer_write_buffer(int table_id, pagenum_t pagenum, page_t * src){
	/*
	printf("buffer_write_buffer started\n");
	printf("buffer_write_buffer pagenum %ld\n", pagenum);
	*/
	int i = 0;
	while(i < num_buffer && buffer_pool[i]->table_id != -1){
		if(buffer_pool[i]->table_id == table_id && buffer_pool[i]->page_num == pagenum){
			printf("buffer_pool[i]->table_id %d \n", buffer_pool[i]->table_id);
			printf("buffer_pool[i]->page_num %ld \n", buffer_pool[i]->page_num);
			buffer_pool[i]->is_dirty = 1;
			for(int j = 0; j < 512; j++){
				buffer_pool[i]->page.buffer[j] = src->buffer[j];
			}
			return 0;
		}
		i++;
	}
	printf("buffer_write_buffer error\n");
	return 0;
}

int buffer_write_disk(int idx){
	/*
	for(int i = 0; i < 10; i++){
		printf("page->buffer[i] %ld\n", buffer_pool[idx]->page.buffer[i]);
	}
			printf("buffer_pool[idx]->table_id %d\n", buffer_pool[idx]->table_id);
			printf("buffer_pool[idx]->page_num %ld\n", buffer_pool[idx]->page_num);
			*/
	file_write_page(unique_id[buffer_pool[idx]->table_id], buffer_pool[idx]->page_num, &(buffer_pool[idx]->page)); 
	return 0;
}
pagenum_t file_alloc_page_buffer(int table_id){
	pagenum_t new_pagenum;
	new_pagenum = file_alloc_page(unique_id[table_id]);

	// 새로 받은 page를 buffer에 올려
	page_t * c = (page_t *)malloc(sizeof(page_t));
	buffer_read_buffer(table_id, new_pagenum, c);

	// buffer의 header에 있는 free_start를 업데이트
	page_t * header_c = (page_t *)malloc(sizeof(page_t));
	buffer_read_buffer(table_id, 0, header_c);
	header_c->buffer[0] = c->buffer[0];
	buffer_write_buffer(table_id, 0, header_c);

	unpin(table_id, new_pagenum);
	unpin(table_id, 0);

	free(c);
	free(header_c);
	return new_pagenum;
}
void set_free_pages_buffer(int table_id){

	set_free_pages(unique_id[table_id]);

	// buffer의 header에 있는 free_start를 업데이트
	page_t * header_disk_c = (page_t *)malloc(sizeof(page_t));
	page_t * header_buffer_c = (page_t *)malloc(sizeof(page_t));
	buffer_read_buffer(table_id, 0, header_buffer_c);
	file_read_page(unique_id[table_id], 0, header_disk_c);
	printf("header_disk_c->buffer[2] %ld\n", header_disk_c->buffer[2]);
	header_buffer_c->buffer[2] = header_disk_c->buffer[2];
	buffer_write_buffer(table_id, 0, header_buffer_c);

	unpin(table_id, 0);

	free(header_disk_c);
	free(header_buffer_c);
}

