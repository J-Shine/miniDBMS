#ifndef __BPT_H_
#define __BPT_H_

#include <stdint.h>


int init_db(int buf_num);
int open_table(char *pathname);
int db_insert(int table_id, int64_t key, char *value);
int db_find(int table_id, int64_t key, char *ret_val, int trx_id);
int db_delete(int table_id, int64_t key);
int db_update(int table_id, int64_t key, char* value, int trx_id);
int close_table(int table_id);
int shutdown_db(void);
int trx_begin(void);
int trx_commit(int trx_id);

#endif
