#include "bptree.h"

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
extern int fd;
leaf_page_t * find_leaf(int64_t key) {
	/*
	page_t * page1 = (page_t *)malloc(sizeof(page_t));
	fd = open("sample_10000.db", O_CREAT|O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	file_read_page(0,page1->buffer);
	printf("file_read_page buffer: %ld", page1->buffer[0]);
	*/
    // get header page
    page_t * c = (page_t *)malloc(sizeof(page_t));
    file_read_page(0, c);
    header_page_t * header_page = (header_page_t *)malloc(sizeof(header_page_t));
    header_page = (header_page_t *)c;
    printf("checking header_page\n");
    // check header page
    printf("%ld\n", header_page->free_start);
    printf("%ld\n", header_page->root_start);
    printf("%ld\n", header_page->num_pages);

    if (header_page->root_start == 0){
    	printf("returning NULL\n");
        return NULL;
    }
    file_read_page(header_page->root_start, c);
    internal_page_t * internal_page = (internal_page_t *)malloc(sizeof(internal_page_t));
    internal_page = (internal_page_t *)c;
    while(!internal_page->is_leaf){
	    int i = 0;
	    while(i < internal_page->num_keys){
		    printf("keys: %ld\n", internal_page->key_pagenum[i].key);
		    if(key >= internal_page->key_pagenum[i].key) {
			    i++;
		    }
		    else{
			    i--;
			    break;
		    }
	    }
	    pagenum_t next_page_num = internal_page->key_pagenum[i].pagenum;
	    file_read_page(next_page_num, c);
	    internal_page = (internal_page_t *)c;
    }
    leaf_page_t * leaf_page = (leaf_page_t *)malloc(sizeof(leaf_page_t));
    leaf_page = (leaf_page_t *)internal_page;
    int i = 0;
    while(i < leaf_page->num_keys){
	    printf("key|value: %ld | %s\n", leaf_page->key_value[i].key, leaf_page->key_value[i].value);
	    i++;
    }
    printf("returning leaf_page\n");
    return leaf_page;
}


/* Finds and returns the record to which
 * a key refers.
 */
int find(int64_t key, char * ret) {
    int i = 0;
    leaf_page_t * c = find_leaf(key);
    if (c == NULL) return -1;
    for (i = 0; i < c->num_keys; i++)
        if (c->key_value[i].key == key) break;
    if (i == c->num_keys) 
        return -1;
    else{
	strncpy(ret, c->key_value[i].value, sizeof(c->key_value[i].value));
        return 0;
    }
}

