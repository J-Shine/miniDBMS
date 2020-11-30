#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <lock_table.h>
#include <trx.h>
#include <pthread.h>

#define TABLE_SIZE 100
#define KEY_SIZE 10000
#define HASH_SIZE TABLE_SIZE * KEY_SIZE 

int hash(int table_id, int64_t key){
	table_id = table_id % TABLE_SIZE;
	key = key % KEY_SIZE;

	int output = table_id * KEY_SIZE;
	output = output + key;
	return output;
}

// lock_table_latch 초기화
pthread_mutex_t lock_table_latch = PTHREAD_MUTEX_INITIALIZER; 

hash_t * hash_array[HASH_SIZE];
pthread_cond_t cond_array[HASH_SIZE];

int init_lock_table()
{
	// hash table 동적할당
	for(int i = 0; i < HASH_SIZE; i++){
		hash_array[i] = (hash_t*)malloc(sizeof(hash_t));
		hash_array[i]->table_id = -1;
		hash_array[i]->key = -1;
		hash_array[i]->tail = NULL;
		hash_array[i]->head = NULL;
		hash_array[i]->next = NULL;
	}
	return 0;
}

lock_t* lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode)
{
    //printf("start lock_acquire()\n");
	/* get hashed key */
	int hashed_key = hash(table_id, key);

	/* create and initialize lock_object */
	lock_t * lock_object = (lock_t *)malloc(sizeof(lock_t));
	lock_object->prev = NULL;
	lock_object->next = NULL;
	lock_object->hashed_key = hashed_key;
	lock_object->cond = &(cond_array[hashed_key]);
    lock_object->lock_mode = lock_mode;
    lock_object->trx_next_lock = NULL;
    lock_object->trx_owner_id = trx_id;
    lock_object->lock_acquired = false;

	pthread_mutex_lock(&lock_table_latch); // start critical section
    
    /* find my trx's acquired lock */
    /* match trx from trx's list */
    int acquired_cnt = 0;
    int prev_lock_acquired_cnt = 0;
    int acquired_hashed_key[20];
    int k = 0;
    trx_t * trx_iter_self = trx_head;
    bool trx_found_self = false;
    bool lock_need_wait = false;
    while(trx_iter_self != NULL && trx_found_self == false){
        if(trx_iter_self->trx_id == trx_id){
            //printf("q\n");
            trx_found_self = true;
        }
        else{
            //printf("p\n");
            trx_iter_self = trx_iter_self->trx_next;
        }
    }
    /* find my acquired locks */
    lock_t * lock_iter_self;
    if(trx_iter_self != NULL && trx_iter_self->lock_head != NULL){
        lock_iter_self = trx_iter_self->lock_head;
    }
    else{
        lock_iter_self = NULL;
    }

    while(lock_iter_self != NULL){
        if(lock_iter_self->lock_acquired){
            acquired_hashed_key[k++] = lock_iter_self->hashed_key;
            acquired_cnt++;
        }
        else
            lock_iter_self = lock_iter_self->trx_next_lock;
    }
    //printf("deadlock detect start\n");

    /* check deadlock */
    if(hash_array[hashed_key]->tail != NULL){
        lock_t * prev_lock_object = hash_array[hashed_key]->tail->prev; 
        bool deadlock_found = false;
        /* check prev lock lists */
        while(prev_lock_object != NULL && deadlock_found == false){
            if(prev_lock_object->lock_acquired == true){
               prev_lock_acquired_cnt++; 
               if(prev_lock_object->lock_mode == 1){
                   lock_need_wait = true;
               }
            }
            int owner_id = prev_lock_object->trx_owner_id;
            trx_t * trx_iter = trx_head;
            lock_t * lock_iter;
            bool trx_found = false;
            /* find trx from trx_list */
            while(trx_iter != NULL && trx_found == false && deadlock_found == false){
                if(trx_iter->trx_id == owner_id){
                    lock_iter = trx_iter->lock_head;
                    trx_found = true;
                }
                if(trx_found == false)
                    trx_iter = trx_iter->trx_next;
            }
            /* check trx's lock list */
            while(lock_iter != NULL && deadlock_found == false){
                if(lock_iter->lock_acquired == false){
                    for(int i = 0; i < acquired_cnt && deadlock_found == false; i++){
                        if(lock_iter->hashed_key == acquired_hashed_key[i])
                            deadlock_found = true;
                    }
                }
                lock_iter = lock_iter->trx_next_lock;
            }
            prev_lock_object = prev_lock_object->prev;
        }

        if(prev_lock_acquired_cnt > 0 && lock_mode == 1){
            lock_need_wait = true;
        }

        /* abort */
        if(deadlock_found == true){
            //printf("mutex unlock lock_acquire\n");
	        pthread_mutex_unlock(&lock_table_latch); // start critical section
            //printf("end lock_acquire()1\n");
            return NULL;
        }
    }
    //printf("deadlock detect done\n");

	/* case that no one acquired lock */
	if(hash_array[hashed_key]->head == NULL && lock_need_wait == false){

		hash_array[hashed_key]->tail = lock_object;
		hash_array[hashed_key]->head = lock_object;
		pthread_cond_init(lock_object->cond, 0);
	}

	/* case that only one already acquired lock */
	else if(hash_array[hashed_key]->head != NULL 
			&& hash_array[hashed_key]->head->next == NULL
			&& hash_array[hashed_key]->head->prev == NULL){
		lock_object->prev = hash_array[hashed_key]->head;
		hash_array[hashed_key]->head->next = lock_object;
		hash_array[hashed_key]->tail = lock_object;
		if(lock_need_wait == true){
            pthread_cond_wait(lock_object->cond, &lock_table_latch); 
        }
	}
	
	/* case that someone already acquired lock */
	else {
		hash_array[hashed_key]->tail->next = lock_object;
		lock_object->prev = hash_array[hashed_key]->tail;
		hash_array[hashed_key]->tail = lock_object;
		if(lock_need_wait == true){
            pthread_cond_wait(lock_object->cond, &lock_table_latch); 
        }
	}

	hash_array[hashed_key]->table_id = table_id;
	hash_array[hashed_key]->key= key;
    
    lock_object->lock_acquired = true;

	pthread_mutex_unlock(&lock_table_latch); // end critical section
	return lock_object;
}

int lock_release(lock_t* lock_obj)
{
	/* GOOD LUCK !!! */
	pthread_mutex_lock(&lock_table_latch); // start critical section

	/* get hashed key */
	int hashed_key = lock_obj->hashed_key;
	
	/* remove lock_obj from the list */
	/* case that only one object left */
	if(hash_array[hashed_key]->head == NULL){
		printf("이상하다? head가 NULL이네?\n");
		return 0;
	}

	/* case that is only one listed */
	if(lock_obj->next == NULL && lock_obj->prev == NULL){
		//free(lock_obj->cond);
		hash_array[hashed_key]->head = NULL;
		hash_array[hashed_key]->tail = NULL;
	}

	/* case that is not head nor tail */
	else if(lock_obj->next != NULL && lock_obj->prev != NULL){
		lock_obj->prev->next = lock_obj->next;
		lock_obj->next->prev = lock_obj->prev;
	}
	
	/* case that is head and not the only one */
	else if(lock_obj->next != NULL && lock_obj->prev == NULL){
		hash_array[hashed_key]->head = lock_obj->next;
		hash_array[hashed_key]->head->prev = NULL;
	}
	
	/* case that is tail and not the only one */
	else{
		hash_array[hashed_key]->tail = lock_obj->prev;
		hash_array[hashed_key]->tail->next = NULL;
	}

	hash_array[hashed_key]->table_id = -1;
	hash_array[hashed_key]->key = -1;

	pthread_cond_signal(lock_obj->cond);
	free(lock_obj);
	pthread_mutex_unlock(&lock_table_latch); // end critical section
	return 0;
}
