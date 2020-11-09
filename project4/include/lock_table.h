#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include <stdint.h>

typedef struct lock_t lock_t;
typedef struct hash_t hash_t;

/* hash function */
int hash(int table_id, int64_t key);

/* APIs for lock table */
int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key);
int lock_release(lock_t* lock_obj);

#endif /* __LOCK_TABLE_H__ */
