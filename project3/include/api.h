// api.h

#define _GNU_SOURCE
#include "bptree.h"
#include<string.h>


extern int init_db(int num_buf);

extern int open_table(char * pathname);

int db_insert(int table_id, int64_t key, char * value);

int db_find(int table_id, int64_t key, char * ret_val);

int db_delete(int table_id, int64_t key);

int close_table(int table_id);

extern int shutdown_db(void);



