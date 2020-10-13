// index.c

#include "index.h"

int open_table(char * pathname){
	if((fd = open(pathname, O_CREAT|O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO)) == -1){
		perror("open");
		return -1;
	}
	return fd;
}

int db_insert(int64_t key, char * value){
}

int db_find(int64_t key, char * ret_val){
	//char temp[120];
	if((find(key, ret_val)) < 0){
		return -1;
	}
	//ret_val = strcpy(ret_val, temp);
	printf("db_find inside: %s\n", ret_val);
	return 0;
}

int db_delete(int64_t key){
}
