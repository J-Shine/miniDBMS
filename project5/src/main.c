#include "api.h"
#include<errno.h>

// MAIN

int main( int argc, char ** argv ) {

    char * input_file;
    page_t * root;
    char path[120];
    int64_t key;
    char value[120];
    char instruction;

    root = NULL;

    page_t * pages;
    
    int num_buf = 0;
    int64_t cnt = 1;
    int tid = 0;
    int trx_id = 0;
    char ret_value[120];
    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
	case 'o':
        init_db(100);
        scanf("%s", path);
		if(open_table(path) < 0){ 
			printf("Open Failed\n"); 
			return 1; 
		}
        trx_begin();

		break;
	case 'f':
		scanf("%d", &tid);
		scanf("%ld", &key);
		scanf("%d", &trx_id);
		if((db_find(tid, key, ret_value, trx_id)) < 0){
			printf("key|value not found\n");
			break;
		}
		printf("key|value found: %ld | %s\n", key, ret_value);
		break;
        case 'p':
		scanf("%d", &tid);
                print_buffer();
		print_LRU_list();
		print_tree(tid);
                break;
	case 's':
		scanf("%d", &tid);
		scanf("%ld", &key);
		scanf("%s", value);
		scanf("%d", &trx_id);
		db_insert(tid, key, value, trx_id);
		break;
		/*
        case 'd':
            	scanf("%ld", &key);
            	db_delete(key);
            	break;
        case 't':
                print_tree();
                break;
		*/
	case 'c':
		scanf("%d", &tid);
		close_table(tid);
		break;
	case 'b':
		scanf("%d", &num_buf);
		init_db(num_buf);
		break;
	case 'q':
		shutdown_db();
		return 0;
		break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    return EXIT_SUCCESS;
}
