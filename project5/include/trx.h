#include<lock_table.h>

struct hist_t {
    int table_id;
    int key;
    char * before;
    char * after;
    struct hist_t * prev;
    struct hist_t * next;
};

typedef struct hist_t hist_t;

struct trx_t {
    int trx_id;
    trx_t * trx_next;
    lock_t * lock_head;
    hist_t * upd_hist_head;
    hist_t * upd_hist_tail;
};

typedef struct trx_t trx_t;

extern int g_trx_id;
extern trx_t * trx_head;
extern trx_t * trx_tail;

void* trx_start();

int trx_make_id();

int trx_clean(int trx_id, int clean_mode);
