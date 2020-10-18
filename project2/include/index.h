// index.h

#define _GNU_SOURCE
#include "bptree.h"
#include<string.h>

extern int open_table(char * pathname);

int db_insert(int64_t key, char * value);

int db_find(int64_t key, char * ret_val);

int db_delete(int64_t key);



