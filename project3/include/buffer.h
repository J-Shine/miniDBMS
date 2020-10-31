#include "file.h"

struct buffer_t{
	page_t page; // 4096byte
	int64_t table_id;
	int64_t page_num;
	int is_dirty;
	int is_pinned;
	struct buffer_t * LRU_next;
	int64_t key; // 8byte
	char value[120]; // 120byte
};	
typedef struct buffer_t buffer_t;
