#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct trx_t trx_t;

struct lock_t {
	struct lock_t * prev;
	struct lock_t * next;
	int hashed_key;
	pthread_cond_t * cond;
    int lock_mode;
    struct lock_t * trx_next_lock;
    int trx_owner_id;
    bool lock_acquired;
};

typedef struct lock_t lock_t;

struct hash_t {
	int table_id;
	int64_t key;
	struct lock_t * tail;
	struct lock_t * head;
	struct hash_t * next;
};

typedef struct hash_t hash_t;

/* hash function */
int hash(int table_id, int64_t key);

/* APIs for lock table */
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode);
int lock_release(lock_t* lock_obj);

#endif /* __LOCK_TABLE_H__ */
