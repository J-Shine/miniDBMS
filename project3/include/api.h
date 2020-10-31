// api.h

#define _GNU_SOURCE
#include "bptree.h"
#include<string.h>

extern buffer_t ** buffer_pool;
extern int num_buffer;

int init_db(int num_buf);

extern int open_table(char * pathname);

int db_insert(int table_id, int64_t key, char * value);

int db_find(int table_id, int64_t key, char * ret_val);

int db_delete(int table_id, int64_t key);

int close_table(int table_id);

int shutdown_db(void);



