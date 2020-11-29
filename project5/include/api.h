// api.h

#define _GNU_SOURCE
#include <string.h>
#include <bptree.h>


extern int init_db(int num_buf);

extern int open_table(char * pathname);

int db_insert(int table_id, int64_t key, char * value, int trx_id);

int trx_begin();

int trx_commit(int trx_id);

int db_find(int table_id, int64_t key, char * ret_val, int trx_id);

int db_update(int table_id, int64_t key, char* values, int trx_id);

int db_delete(int table_id, int64_t key);

int close_table(int table_id);

extern int shutdown_db(void);



