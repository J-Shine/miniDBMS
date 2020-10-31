#include "api.h"
#include<errno.h>

// MAIN


int main( int argc, char ** argv ) {

    char * input_file;
    page_t * root;
    char * path;
    int64_t key;
    char value[120];
    char instruction;

    root = NULL;

    page_t * pages;
    
    int num_buf = 0;
    int64_t cnt = 1;
    char ret_value[120];
    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
	case 'o':
		path = (char *)malloc(sizeof(char) * 21);
		scanf("%s", path);
		if(open_table(path) < 0){ 
			printf("Open Failed\n"); 
			return 1; 
		}
		free(path);
		break;
	case 'f':
		scanf("%ld", &key);
		if((db_find(key, ret_value)) < 0){
			printf("key|value not found\n");
		}
		printf("key|value found: %ld | %s\n", key, ret_value);
		break;
	case 'i':
		//while(cnt < 33){
		//	key = cnt; value[0] = 'a';
		//	db_insert(key, value);
		//	cnt++;
		//}
		scanf("%ld", &key);
		scanf("%s", value);
		db_insert(key, value);
		break;
        case 'd':
            	scanf("%ld", &key);
            	db_delete(key);
            	break;
        case 't':
                print_tree();
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
