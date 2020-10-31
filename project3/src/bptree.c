#include "bptree.h"

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
extern int fd;

// set header if doesn't exist
void set_header(){
	page_t * c = (page_t *)malloc(sizeof(page_t));
	file_read_page(0, c);
	header_page_t * header = (header_page_t *)c;
	header->free_start = 0;
	header->root_start = 0;
	header->num_pages = 1;
	/*
	printf("set_header %ld\n", header->free_start);
	printf("set_header %ld\n", header->root_start);
	printf("set_header %ld\n", header->num_pages);
	*/
	c = (page_t *)header;
	file_write_page(0, c);
	/*
	printf("c %ld\n", c->buffer[0]);
	printf("c %ld\n", c->buffer[1]);
	printf("c %ld\n", c->buffer[2]);
	*/


	page_t * c2 = (page_t *)malloc(sizeof(page_t));
	file_read_page(0, c2);
	header_page_t * header2 = (header_page_t *)malloc(sizeof(header_page_t));
	header2 = (header_page_t *)c2;
	/*
	printf("read_header %ld\n", header2->free_start);
	printf("set_header %ld\n", header2->root_start);
	printf("set_header %ld\n", header2->num_pages);
	*/


	free(c);
}

// set free pages
void set_free_pages(){
	page_t * header = (page_t *)malloc(sizeof(page_t));
	page_t * free_page1 = (page_t *)malloc(sizeof(page_t));
	//printf("inside set_free_pagess\n");
	
	// get current free number
	file_read_page(0, header);

	if(header->buffer[0] == 0)
		header->buffer[0] = 1;
	int cnt = 1;
	int64_t next_free_num = header->buffer[0];
	while(cnt < 5 && next_free_num != 0){
		file_read_page(next_free_num, free_page1);
		next_free_num = free_page1->buffer[0];
		//printf("next_free_numm %ld \n", next_free_num);
		cnt++;
	}
	while(cnt < 5){
		file_free_page(header->buffer[2]);
		header->buffer[2] = header->buffer[2] + 1;
		cnt++;
	}
	/*
	printf("headerr: free_start %ld\n", header->buffer[0]);
	printf("headerr: root_start %ld\n", header->buffer[1]);
	printf("headerr: num_pages %ld\n", header->buffer[2]);
	*/
	file_write_page(0, header);
}

/* Finds and returns the record to which
 * a key refers.
 */
int find(int file_descriptor, int64_t key, char * ret) {
    int i = 0;
    pagenum_t * pagenum = (pagenum_t *)malloc(sizeof(pagenum_t));
    leaf_page_t * leaf = (leaf_page_t *)malloc(sizeof(leaf_page_t));
    int c = find_leaf(file_descriptor, key, leaf, pagenum);
    if (c == -1) {
	    free(pagenum);
	    free(leaf);
	    return -1;
    }
    for (i = 0; i < leaf->num_keys; i++)
        if (leaf->key_value[i].key == key) break;
    if (i == leaf->num_keys) {
	    free(pagenum);
	    free(leaf);
            return -1;
    }

    else{
	strncpy(ret, leaf->key_value[i].value, sizeof(leaf->key_value[i].value));
	free(pagenum);
	free(leaf);
        return 0;
    }
    free(pagenum);
    free(leaf);
    return 0;
}

int find_leaf(int file_descriptor, int64_t key, leaf_page_t * leaf_page, pagenum_t * pagenum) {

    // get header page
    page_t * header_c = (page_t *)malloc(sizeof(page_t));
    /////////////////////////////////////////////////////////////
    file_read_page(0, header_c);
    header_page_t * header_page = (header_page_t *)header_c;
    //printf("checking header_page\n");
    // check header page
    /*
    printf("%ld\n", header_page->free_start);
    printf("%ld\n", header_page->root_start);
    printf("%ld\n", header_page->num_pages);
    */

    if (header_page->root_start == 0){
        return -1; 
    }
    *pagenum = header_page->root_start;
    page_t * internal_c = (page_t *)malloc(sizeof(page_t));
    /////////////////////////////////////////////////////////////
    file_read_page(header_page->root_start, internal_c);
    internal_page_t * internal_page = (internal_page_t *)internal_c;

    while(!internal_page->is_leaf){
	    
	    int i = 0;
	    while(i < internal_page->num_keys){
		    /*
		    printf("i %d\n", i);
		    printf("key %ld\n", key);
		    printf("key_pagenum %ld\n", internal_page->key_pagenum[i + 1].key);
		    */
		    if(key < internal_page->key_pagenum[i].key){
			    break;
		    }
		    if(key >= internal_page->key_pagenum[i].key) {
			    i++;
		    }
	    }
	    if(i == 0)
		    *pagenum = internal_page->one_more_pagenum;
	    else
		    *pagenum = internal_page->key_pagenum[i - 1].pagenum;
	    
	    /////////////////////////////////////////////////////////////
	    file_read_page(*pagenum, internal_c);
	    internal_page = (internal_page_t *)internal_c;
    }
    internal_c = (page_t *)internal_page;
    page_t * leaf_c = (page_t *)leaf_page;
    copy_page_t(leaf_c, internal_c);
    leaf_page = (leaf_page_t *)leaf_c;
    int i = 0;
    while(i < leaf_page->num_keys){
	    //printf("key|value: %ld | %s\n", leaf_page->key_value[i].key, leaf_page->key_value[i].value);
	    i++;
    }
    //printf("returning leaf_page\n");
    free(header_c);
    free(internal_c);
    return 0;
}

