#include <stdio.h>
#include <stdlib.h>
#include <lock_table.h>
#include <pthread.h>

struct lock_t {
	int num;
	lock_t * prev;
	lock_t * next;
	int hashed_key;
	pthread_cond_t * cond;
};

typedef struct lock_t lock_t;

struct hash_t {
	int table_id;
	int64_t key;
	lock_t * tail;
	lock_t * head;
};

typedef struct hash_t hash_t;

int hash(int table_id, int64_t key){
	if(table_id * key > 1000){
		printf("key is too big Error!\n");
		return -1;
	}
	int output = table_id * 100;
	output = output + key;
	return output;
}

// lock_table_latch 초기화
pthread_mutex_t lock_table_latch = PTHREAD_MUTEX_INITIALIZER; 

hash_t * hash_array[1000];
pthread_cond_t cond_array[1000];

int init_lock_table()
{
	// hash table 동적할당
	//hash_array = (hash_t **)malloc(sizeof(hash_t));
	for(int i = 0; i < 1000; i++){
		hash_array[i] = (hash_t*)malloc(sizeof(hash_t));
		hash_array[i]->table_id = -1;
		hash_array[i]->key = -1;
		hash_array[i]->tail = NULL;
		hash_array[i]->head = NULL;
	}
	/*
	for(int i = 0; i < 1000; i++){
		cond_array[i] = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	}
	*/

	return 0;
}
int i = 1;

lock_t* lock_acquire(int table_id, int64_t key)
{
	/* get hashed key */
	int hashed_key = hash(table_id, key);

	/* create and initialize lock_object */
	lock_t * lock_object = (lock_t *)malloc(sizeof(lock_t));
	lock_object->num = i++;
	lock_object->prev = NULL;
	lock_object->next = NULL;
	lock_object->hashed_key = hashed_key;
	lock_object->cond = &(cond_array[hashed_key]);

	/* ENJOY CODING !!!! */
	pthread_mutex_lock(&lock_table_latch); // start critical section
	//printf("get mutex on lock_acquire() table_id %d, key %ld\n", table_id, key);

	//lock_t * next;
	/* case that no one acquired lock */
	if(hash_array[hashed_key]->head == NULL){
		//printf("working on table_id %d, key %ld\n", table_id, key);
		/*
		hash_array[hashed_key]->tail = (hash_t *)malloc(sizeof(hash_t));
		hash_array[hashed_key]->head = (hash_t *)malloc(sizeof(hash_t));
		*/
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
		/*
		printf("sleep start table_id %d, key %ld\n", table_id, key);
		next = hash_array[hashed_key]->head;
		while(next != NULL){
			printf("lock object of table_id %d, key %ld: %d\n", table_id, key, next->num);
			next = next->next;
		}
		*/
		pthread_cond_wait(lock_object->cond, &lock_table_latch); 
		/*
		printf("sleep end table_id %d, key %ld\n", table_id, key);
		next = hash_array[hashed_key]->head;
		while(next != NULL){
			printf("lock object of table_id %d, key %ld: %d\n", table_id, key, next->num);
			next = next->next;
		}
		*/
	}

	
	/* case that someone already acquired lock */
	else {
		hash_array[hashed_key]->tail->next = lock_object;
		lock_object->prev = hash_array[hashed_key]->tail;
		hash_array[hashed_key]->tail = lock_object;
		/*
		printf("sleep start table_id %d, key %ld\n", table_id, key);
		next = hash_array[hashed_key]->head;
		while(next != NULL){
			printf("lock object of table_id %d, key %ld: %d\n", table_id, key, next->num);
			next = next->next;
		}
		*/
		pthread_cond_wait(lock_object->cond, &lock_table_latch); 
		/*
		printf("sleep end table_id %d, key %ld\n", table_id, key);
		next = hash_array[hashed_key]->head;
		while(next != NULL){
			printf("lock object of table_id %d, key %ld: %d\n", table_id, key, next->num);
			next = next->next;
		}
		*/
	}

	hash_array[hashed_key]->table_id = table_id;
	hash_array[hashed_key]->key= key;

	pthread_mutex_unlock(&lock_table_latch); // end critical section
	//printf("return mutex on lock_acquire() table_id %d, key %ld\n", table_id, key);
	return lock_object;
}

int lock_release(lock_t* lock_obj)
{
	/* GOOD LUCK !!! */
	pthread_mutex_lock(&lock_table_latch); // start critical section

	/* get hashed key */
	int hashed_key = lock_obj->hashed_key;
	//printf("release hashed_key %d, lock_object %d\n", lock_obj->hashed_key, lock_obj->num); 
	
	/* remove lock_obj from the list */
	/* case that only one object left */
	if(hash_array[hashed_key]->head == NULL){
		printf("이상하다? head가 NULL이네?\n");
		return 0;
	}

	/* case that is only one listed */
	if(lock_obj->next == NULL && lock_obj->prev == NULL){
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
	//pthread_exit(NULL);
	return 0;
}
