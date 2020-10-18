#include "index.h"
#include<errno.h>

// MAIN


int main( int argc, char ** argv ) {
	/*
	page_t * page1 = (page_t *)malloc(sizeof(page_t));
	fd = open("sample_10000.db", O_CREAT|O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	file_read_page(0,page1);
	printf("file_read_page buffer: %ld\n", page1->buffer[0]);
	printf("file_read_page buffer: %ld\n", page1->buffer[1]);
	printf("file_read_page buffer: %ld\n", page1->buffer[2]);
	pread(fd, page1, 4096, 0); 
	printf("pread buffer: %ld\n", page1->buffer[0]);
	printf("pread buffer: %ld\n", page1->buffer[1]);
	printf("pread buffer: %ld\n", page1->buffer[2]);
	close(fd);
	page_t * page2 = (page_t *)malloc(sizeof(page_t));
	pread(fd, page2, 4096, 0); 
	printf("pread buffer after close: %ld\n", page2->buffer[0]);
	printf("pread buffer after close: %ld\n", page2->buffer[1]);
	printf("pread buffer after close: %ld\n", page2->buffer[2]);

	return 0;
	
	*/

	/*
	page_t * page1;
	//header_page_t ** header = (header_page_t **) &page1;

	if((fd = open("free", O_CREAT|O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO)) == -1){
		perror("open");
		exit(1);
	}
	uint64_t initialize = 1234;
	pwrite(fd, &initialize, 4096, 0); 
	pread(fd, page1->buffer, 4096, 0); 
	
	  
	uint64_t first = 1;
	uint64_t second = 2;
	uint64_t third = 3;
	file_free_page(first);
	file_free_page(second);
	file_free_page(third);
	//printf("%lu\n", file_alloc_page());
	//printf("%lu\n", file_alloc_page());
	//printf("%lu\n", file_alloc_page());
	//printf("%lu\n", file_alloc_page());

	file_read_page(0, page1);
	*/
	//return 0;



    char * input_file;
    page_t * root;
    char * path = "debug";
    int64_t key;
    char value[120];
    char instruction;

    root = NULL;
    //verbose_output = false;

    /*
    if (argc > 1) {
        order = atoi(argv[1]);
        if (order < MIN_ORDER || order > MAX_ORDER) {
            fprintf(stderr, "Invalid order: %d .\n\n", order);
            usage_3();
            exit(EXIT_FAILURE);
        }
    }

    license_notice();
    usage_1();  
    usage_2();

    if (argc > 2) {
        input_file = argv[2];
        fp = fopen(input_file, "r");
        if (fp == NULL) {
            perror("Failure  open input file.");
            exit(EXIT_FAILURE);
        }
        while (!feof(fp)) {
            fscanf(fp, "%d\n", &input);
            root = insert(root, input, input);
        }
        fclose(fp);
        print_tree(root);
    }
    */
    page_t * pages;

    int64_t cnt = 1;
    int td;
    char ret_value[120];
    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
	case 'o':
		if((td = dup(open_table(path))) < 0){ 
			printf("Open Failed\n"); 
			return 1; 
		}
	       	printf("fd: %d\n", td);
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
	/*
        case 'i':
            scanf("%ld", &key);
            scanf("%s", value);
            root = insert(root, key, value);
            print_tree(root);
            break;
        case 'f':
            scanf("%ld", &key);
            find_and_print(root, key, instruction == 'p');
            break;
        case 'q':
            while (getchar() != (int)'\n');
            return EXIT_SUCCESS;
            break;
        case 't':
            print_tree(root);
            break;
        case 'v':
            verbose_output = !verbose_output;
            break;
        default:
            usage_2();
            break;
	*/
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    return EXIT_SUCCESS;
}
