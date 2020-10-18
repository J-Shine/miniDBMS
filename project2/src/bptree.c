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
int find(int64_t key, char * ret) {
    int i = 0;
    pagenum_t * pagenum = (pagenum_t *)malloc(sizeof(pagenum_t));
    leaf_page_t * leaf = (leaf_page_t *)malloc(sizeof(leaf_page_t));
    int c = find_leaf(key, leaf, pagenum);
    if (c == -1) {
	    free(pagenum);
	    free(leaf);
	    return -1;
    }
    //printf("leaf->num_keyssss %d\n", leaf->num_keys); 
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

int find_leaf(int64_t key, leaf_page_t * leaf_page, pagenum_t * pagenum) {
	/*
	page_t * page1 = (page_t *)malloc(sizeof(page_t));
	fd = open("sample_10000.db", O_CREAT|O_RDWR|O_SYNC, S_IRWXU|S_IRWXG|S_IRWXO);
	file_read_page(0,page1->buffer);
	printf("file_read_page buffer: %ld", page1->buffer[0]);
	*/
    // get header page
    page_t * header_c = (page_t *)malloc(sizeof(page_t));
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
    //printf("pagenum inside %ld\n", *pagenum);
    page_t * internal_c = (page_t *)malloc(sizeof(page_t));
    file_read_page(header_page->root_start, internal_c);
    internal_page_t * internal_page = (internal_page_t *)internal_c;
    //printf("hhhhhh->root_start: %ld\n", header_page->root_start);

    while(!internal_page->is_leaf){
	    int i = -1;
	    while(i < internal_page->num_keys - 1){
		    if(key < internal_page->key_pagenum[i + 1].key && i == -1){
			    break;
		    }
		    if(key >= internal_page->key_pagenum[i + 1].key) {
			    i++;
		    }
	    }
	    if(i == -1)
		    *pagenum = internal_page->one_more_pagenum;
	    else
		    *pagenum = internal_page->key_pagenum[i].pagenum;
	    
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

// Insertion

int insert(int64_t key, char * value) {
	page_t * c1 = (page_t *)malloc(sizeof(page_t));
	file_read_page(0, c1);
	header_page_t * head = (header_page_t *)c1;

    /* The current implementation ignores
     * duplicates.
     */

    if (find(key, value) == 0){
	    free(c1);
	    return -1;
    }

    /* Case: the tree does not exist yet.
     * Start a new tree.
     */

    if (head->root_start == 0){ 

        if((start_new_tree(key, value)) < 0){
    		//printf("ii2\n");
	    	free(c1);
		return -1;
	}
    	//printf("ii3\n");
	free(c1);
	return 0;
    }

    /* Case: the tree already exists.
     * (Rest of function body.)
     */

    leaf_page_t * leaf = (leaf_page_t *)malloc(sizeof(leaf_page_t));
    pagenum_t * pagenum = (pagenum_t *)malloc(sizeof(pagenum_t));
    *pagenum = 0;
    find_leaf(key, leaf, pagenum);


    /* Case: leaf has room for key and pointer.
     */
    // record가 29개 이하면 넣고 끝낸다.
    if (leaf->num_keys < LEAF_ORDER - 2) {
	    //printf("nnn: %d\n", leaf->num_keys);
        if((insert_into_leaf(leaf, key, value, pagenum)) < 0){
		free(leaf);
		free(pagenum);
	    	free(c1);
		return -1;
	}
	free(leaf);
	free(c1);
	free(pagenum);
	return 0;
    }

    /* Case:  leaf must be split.
     */
    // record가 30개면 넣고 split

    //printf("head->free_start before the insert_into_leaf_after_splittign: %ld\n", head->free_start);
    insert_into_leaf_after_splitting(leaf, key, value, leaf->parent_pagenum, *pagenum);
    //printf("after the insert_into_leaf_after_splitting\n");
    free(leaf);
    free(pagenum);
    return 0;
}


/* First insertion:
 * start a new tree.
 */
int start_new_tree(int64_t key, char * value) {

        // get new root num
        pagenum_t new_root = file_alloc_page();
    	set_free_pages();
	// read header page
	page_t * c = (page_t *)malloc(sizeof(page_t));
	file_read_page(0, c);
	header_page_t * header = (header_page_t *)c;

	header->root_start = new_root;


    page_t * c_leaf = (page_t *)malloc(sizeof(page_t));
    file_read_page(header->root_start, c_leaf);
    leaf_page_t * leaf_page = (leaf_page_t *)c_leaf;

    // set leaf_page
    leaf_page->parent_pagenum = 0;
    leaf_page->is_leaf = 1;
    leaf_page->num_keys = 1;
    leaf_page->right_sib_pagenum = 0;
    
    // save record to leaf
    leaf_page->key_value[0].key = key;
    strncpy(leaf_page->key_value[0].value, value, sizeof(value));

    // write root_page to disk
    c_leaf = (page_t *)leaf_page;
    file_write_page(header->root_start, c_leaf);
    file_read_page(header->root_start, c_leaf);
    leaf_page = (leaf_page_t *)c_leaf;
    
    // write header_page to disk
    page_t * c_header = (page_t *)header;
    file_write_page(0, c_header);

    free(leaf_page);

    return 0;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
int insert_into_leaf( leaf_page_t * leaf, int64_t key, char * value, pagenum_t * pagenum ) {

    int i, insertion_point;

    insertion_point = 0;
    while (insertion_point < leaf->num_keys && leaf->key_value[insertion_point].key < key){
        insertion_point++;
    }

    for (i = leaf->num_keys; i > insertion_point; i--) {

        leaf->key_value[i].key = leaf->key_value[i - 1].key;
        strncpy(leaf->key_value[i].value, leaf->key_value[i - 1].value, 120);
    }
    leaf->key_value[insertion_point].key = key;
    strncpy(leaf->key_value[insertion_point].value, value, 120);
    leaf->num_keys = leaf->num_keys + 1;
    //printf("inside insert_into_leaf:leaf->num_keys %d\n", leaf->num_keys);
    page_t * c = (page_t *)leaf;

    file_write_page(*pagenum, c);

    return 0;
}

/* Inserts a new key and a new value 
 * into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
int insert_into_leaf_after_splitting(leaf_page_t * leaf, int64_t key, char * value, pagenum_t parent_pagenum, pagenum_t pagenum) {

    leaf_page_t * new_leaf = (leaf_page_t *)malloc(sizeof(page_t));
    int64_t * temp_keys = malloc(LEAF_ORDER * sizeof(int64_t));
    char * temp_values[LEAF_ORDER - 1];
    for(int i = 0; i < LEAF_ORDER - 1; i++){
	    temp_values[i] = (char *)malloc(sizeof(char) * 120);
    }
    int insertion_index, split, i, j;
    int64_t new_key = 0;

    // make leaf
    //printf("before the make_leaf\n");
    pagenum_t * new_pagenum = (pagenum_t *)malloc(sizeof(pagenum_t));
    make_leaf(new_leaf, parent_pagenum, new_pagenum);

    // move records to temp
    insertion_index = LEAF_ORDER - 2;
    //printf("leaf->num_keys %d\n", leaf->num_keys);

    for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
        if (j == insertion_index) j++;
	/*
	printf("leaf->key_value[i].key %ld\n", leaf->key_value[i].key);
	printf("leaf->key_value[i].value %s\n", leaf->key_value[i].value);
	*/
        temp_keys[j] = leaf->key_value[i].key;
        strncpy(temp_values[j], leaf->key_value[i].value, 120);
    }
    //printf("outside the make_leaf1\n");

    // insert new record to temp
    temp_keys[insertion_index] = key;
    strncpy(temp_values[insertion_index], value, 120);
    //printf("outside the make_leaf2\n");

    leaf->num_keys = 0;
    split = cut(LEAF_ORDER - 1);
    //printf("outside the make_leaf3\n");

    for (i = 0; i < split; i++) {
        leaf->key_value[i].key = temp_keys[i];
        strncpy(leaf->key_value[i].value, temp_values[i], 120);
        leaf->num_keys = leaf->num_keys + 1;
    }
    //printf("outside the make_leaf4\n");

    for (i = split, j = 0; i < LEAF_ORDER - 1; i++, j++) {
        new_leaf->key_value[j].key = temp_keys[i];
        strncpy(new_leaf->key_value[j].value, temp_values[i], 120);
        new_leaf->num_keys = new_leaf->num_keys + 1;
	/*
	printf("printf j: %d\n", j);
	printf("temp_values[i] %s\n", temp_values[i]);
	*/
    }

    char zeros[120] = { 0 };
    for (i = leaf->num_keys; i < LEAF_ORDER - 1; i++){
        leaf->key_value[i].key = 0;
    }

    // set right sib
    leaf->right_sib_pagenum = *new_pagenum; 
    new_leaf->right_sib_pagenum = 0;

    // write leaf
    page_t * leaf_c = (page_t *)leaf;
    file_write_page(pagenum, leaf_c);

    // write new_leaf
    page_t * new_leaf_c = (page_t *)new_leaf;
    file_write_page(*new_pagenum, new_leaf_c);


    // insert key into parent
    new_key = new_leaf->key_value[0].key;

    insert_into_parent(leaf_c, new_key, new_leaf_c, parent_pagenum, pagenum, *new_pagenum);

    free(temp_keys);
    for(int i = 0; i < LEAF_ORDER - 1; i++){
	    free(temp_values[i]);
    }
    free(new_pagenum);

    return 0;

}


/* Creates a new leaf by creating a page_t
 * and then adapting it appropriately.
 */
int make_leaf(leaf_page_t * leaf, pagenum_t parent_pagenum, pagenum_t * new_pagenum) {
	// get new page number
	*new_pagenum = file_alloc_page();
	set_free_pages();
	//printf("*new_pagenum %ld\n", *new_pagenum);

	// set leaf
	leaf->parent_pagenum = parent_pagenum;
	leaf->is_leaf = 1;
	leaf->num_keys = 0;

	// write leaf to the disk
	page_t * leaf_c = (page_t *)leaf;
	//printf("*new_pagenum %ld\n", *new_pagenum);
	file_write_page(*new_pagenum, leaf_c);

	return 0;
}


/* Finds the appropriate place to * split a page_t that is too big into two.
 */
int cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}

/* Inserts a new page_t (leaf or internal page_t) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
int insert_into_parent(page_t * left, int64_t key, page_t * right, pagenum_t this_pagenum, pagenum_t left_pagenum, pagenum_t right_pagenum) {

    /* Case: new root. */

    if (this_pagenum == 0){
        insert_into_new_root(left, key, right, left_pagenum, right_pagenum);
    	return 0;
    }

    /* Case: leaf or page_t. (Remainder of
     * function body.)  
     */

    /* Simple case: the new key fits into the page_t. 
     */
    page_t * this_page_c = (page_t *)malloc(sizeof(page_t));
    file_read_page(this_pagenum, this_page_c); 
    internal_page_t * this_page = (internal_page_t *)this_page_c;

    if (this_page->num_keys < INTERNAL_ORDER - 1){
        insert_into_internal(this_page, key, right_pagenum, this_pagenum);
	return 0;
    }


    /* Harder case:  split a page_t in order 
     * to preserve the B+ tree properties.
     */

    pagenum_t parent_pagenum = this_page->parent_pagenum;

    insert_into_internal_after_splitting(this_page, key, right_pagenum, parent_pagenum, this_pagenum);

    free(this_page_c);

    return 0;
}

/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
int insert_into_new_root(page_t * left, int64_t key, page_t * right, pagenum_t left_pagenum, pagenum_t right_pagenum) {

	internal_page_t * new_root = (internal_page_t *)malloc(sizeof(page_t));
	pagenum_t * new_pagenum = (pagenum_t *)malloc(sizeof(pagenum_t));
	make_internal(new_root, 0, new_pagenum);
	new_root->key_pagenum[0].key = key;
	new_root->key_pagenum[0].pagenum = right_pagenum;
	new_root->one_more_pagenum = left_pagenum;
	new_root->num_keys += 1;
	
	// write new root to disk
	page_t * new_root_c = (page_t *)new_root;
	file_write_page(*new_pagenum, new_root_c);
	
	// write new root pagenum to header
	page_t * header_c = (page_t *)malloc(sizeof(page_t));
	file_read_page(0, header_c);
	header_page_t * header = (header_page_t *)header_c;
        header->root_start = *new_pagenum;	
	//printf("----------------------------------------------header->root_start = *new_pagenum %ld \n", *new_pagenum);
	header_c = (page_t *)header;
	file_write_page(0, header_c);
	
	// write new root as parent for left and right
	page_t * child = (page_t *)malloc(sizeof(page_t));

	file_read_page(left_pagenum, child); 
	child->buffer[0] = *new_pagenum;
	file_write_page(left_pagenum, child);

	file_read_page(right_pagenum, child); 
	child->buffer[0] = *new_pagenum;
	file_write_page(right_pagenum, child);


	free(new_root);
	free(new_pagenum);
	free(child);
	return 0;
}

/* Creates a new leaf by creating a page_t
 * and then adapting it appropriately.
 */
int make_internal(internal_page_t * internal, pagenum_t parent_pagenum, pagenum_t * new_pagenum) {
	// get new page number
	*new_pagenum = file_alloc_page();
	set_free_pages();

	// set internal 
	internal->parent_pagenum = parent_pagenum;
	internal->is_leaf = 0;
	internal->num_keys = 0;

	// write leaf to the disk
	page_t * internal_c = (page_t *)internal;
	file_write_page(*new_pagenum, internal_c);

	return 0;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
int insert_into_internal(internal_page_t * internal, int64_t key, pagenum_t pagenum, pagenum_t parent_pagenum) {

    int i, insertion_point;

    insertion_point = 0;
    while (insertion_point < internal->num_keys && internal->key_pagenum[insertion_point].key < key){
        insertion_point++;
    }

    for (i = internal->num_keys; i > insertion_point; i--) {
        internal->key_pagenum[i].key = internal->key_pagenum[i - 1].key;
        internal->key_pagenum[i].pagenum = internal->key_pagenum[i - 1].pagenum;
    }
    internal->key_pagenum[insertion_point].key = key;
    internal->key_pagenum[insertion_point].pagenum = pagenum;
    internal->num_keys = internal->num_keys + 1;
    //printf("inside insert_into_internal internal->num_keys %d \n", internal->num_keys);
    page_t * c = (page_t *)internal;

    file_write_page(parent_pagenum, c);

    return 0;
}


/* Inserts a new key and a new value 
 * into a internal so as to exceed
 * the tree's order, causing the internal to be split
 * in half.
 */
int insert_into_internal_after_splitting(internal_page_t * internal, int64_t key, pagenum_t record_pagenum, pagenum_t parent_pagenum, pagenum_t this_pagenum) {

    internal_page_t * new_internal = (internal_page_t *)malloc(sizeof(page_t));
    int64_t * temp_keys = malloc(INTERNAL_ORDER * sizeof(int64_t));
    pagenum_t * temp_pagenums = malloc(INTERNAL_ORDER * sizeof(int64_t));
    int insertion_index, split, i, j;
    int64_t new_key = 0;

    // make internal 
    //printf("before the make_internal\n");
    pagenum_t * new_pagenum = (pagenum_t *)malloc(sizeof(pagenum_t));
    make_internal(new_internal, parent_pagenum, new_pagenum);

    // move records to temp
    insertion_index = INTERNAL_ORDER - 2;

    for (i = 0, j = 0; i < internal->num_keys; i++, j++) {
        if (j == insertion_index) j++;
        temp_keys[j] = internal->key_pagenum[i].key;
        temp_pagenums[j] = internal->key_pagenum[i].pagenum;
    }
    //printf("outside the make_internal1\n");

    // insert new record to temp
    temp_keys[insertion_index] = key;
    temp_pagenums[insertion_index] = record_pagenum;
    //printf("outside the make_internal2\n");

    internal->num_keys = 0;
    split = cut(INTERNAL_ORDER - 1);
    //printf("outside the make_internal3\n");

    for (i = 0; i < split; i++) {
        internal->key_pagenum[i].key = temp_keys[i];
        internal->key_pagenum[i].pagenum = temp_pagenums[i];
        internal->num_keys = internal->num_keys + 1;
    }

    for (i = split, j = 0; i < INTERNAL_ORDER - 1; i++, j++) {
        new_internal->key_pagenum[j].key = temp_keys[i];
        new_internal->key_pagenum[i].pagenum = temp_pagenums[i];
        new_internal->num_keys = new_internal->num_keys + 1;
	page_t * child = (page_t *)malloc(sizeof(page_t));
	file_read_page(temp_pagenums[i], child);
	child->buffer[0] = *new_pagenum; 
	file_write_page(temp_pagenums[i], child);
	//printf("child->buffer[0] %ld\n", child->buffer[0]);
	free(child);
    }

    for (i = internal->num_keys; i < INTERNAL_ORDER - 1; i++){
        internal->key_pagenum[i].key = 0;
    }
    // print tree용
    internal->right_sib_pagenum = *new_pagenum;
    new_internal->right_sib_pagenum = 0; 

    // write internal 
    page_t * internal_c = (page_t *)internal;
    file_write_page(this_pagenum, internal_c);

    // write new_internal
    page_t * new_internal_c = (page_t *)new_internal;
    file_write_page(*new_pagenum, new_internal_c);




    // insert key into parent
    new_key = new_internal->key_pagenum[0].key;
    parent_pagenum = internal->parent_pagenum;

    insert_into_parent(internal_c, new_key, new_internal_c, parent_pagenum, this_pagenum, *new_pagenum);

    free(temp_keys);
    free(temp_pagenums);
    free(new_pagenum);
    free(new_internal);

    return 0;

}

/* Master deletion function.
 */
int delete(int64_t key) {

    leaf_page_t * key_leaf = (leaf_page_t *)malloc(sizeof(page_t));
    pagenum_t * leaf_pagenum = (pagenum_t *)malloc(sizeof(pagenum_t));
    char rtn_value[120];

    int found_value = find(key, rtn_value);
    int found_leaf = find_leaf(key, key_leaf, leaf_pagenum);
    if (found_value == 0) {
        delete_entry_leaf(*leaf_pagenum, key);
        free(key_leaf);
        free(leaf_pagenum);
	return 0;
    }
    free(key_leaf);
    free(leaf_pagenum);
    return -1;
}

/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
int delete_entry_leaf(pagenum_t leaf_pagenum, int64_t key) {
	//printf("start delete_entry_leaf\n");

    int min_keys;
    page_t * neighbor;
    int neighbor_index;
    int k_prime_index, k_prime;
    int capacity;

    // Remove key and value from leaf.

    remove_entry_from_leaf(leaf_pagenum, key);

    /* Case:  deletion from the root. 
     */
    page_t * header_c = (page_t *)malloc(sizeof(page_t));
    file_read_page(0, header_c);
    header_page_t * header = (header_page_t *)header_c;

    if (header->root_start == leaf_pagenum){ 
	    adjust_root_leaf(leaf_pagenum);
	    free(header_c);
	    //printf("end1 delete_entry_leaf\n");
	    return 0;
    }

	//printf("end2 delete_entry_leaf\n");

    /* Case:  deletion from a page_t below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of page_t,
     * to be preserved after deletion.
     */

    /* Case:  page_t stays at or above minimum.
     * (The simple case.)
     */
	// read leaf
	page_t * leaf_c = (page_t *)malloc(sizeof(page_t));
	file_read_page(leaf_pagenum, leaf_c);
	leaf_page_t * leaf = (leaf_page_t *)leaf_c;

    if (leaf->num_keys > 0)
    	    free(leaf_c);
	    return 0;

    /* Case:  all keys are deleted.
     * merge is needed.
     */

    pagenum_t internal_pagenum = leaf->parent_pagenum;

    delete_entry_internal(internal_pagenum, key);

    free(leaf_c);
    
    return 0;

    /* Find the appropriate neighbor page_t with which
     * to coalesce.
     * Also find the key (k_prime) in the parent
     * between the pointer to page_t n and the pointer
     * to the neighbor.
     */

    /*
    neighbor_index = get_neighbor_index( n );
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    k_prime = n->parent->keys[k_prime_index];
    neighbor = neighbor_index == -1 ? n->parent->pointers[1] : 
        n->parent->pointers[neighbor_index];

    capacity = n->is_leaf ? order : order - 1;

    */

    /*
    if (neighbor->num_keys + n->num_keys < capacity)
        return coalesce_page_ts(root, n, neighbor, neighbor_index, k_prime);
	*/

}

int remove_entry_from_leaf(pagenum_t leaf_pagenum, int64_t key) {

	//printf("start remove_entry_from_leaf\n");
	// read leaf
	page_t * leaf_c = (page_t *)malloc(sizeof(page_t));
	file_read_page(leaf_pagenum, leaf_c);
	leaf_page_t * leaf = (leaf_page_t *)leaf_c;

    char erase_value[120];
    for(int i = 0; i < 120; i++){
	    erase_value[i] = 0;
    }


    // Remove the key & value and shift other keys & values accordingly.
    int i = 0;
    while (leaf->key_value[i].key != key)
        i++;
    for (++i; i < leaf->num_keys; i++){
        leaf->key_value[i - 1].key = leaf->key_value[i].key;
    	strncpy(leaf->key_value[i - 1].value, erase_value, 120);
	strncpy(leaf->key_value[i - 1].value, leaf->key_value[i].value, 120);
    }

    // erase last key and value for tidyness
    
    leaf->key_value[i - 1].key = 0;
    strncpy(leaf->key_value[i - 1].value, erase_value, 120);

    // One key fewer.
    leaf->num_keys = leaf->num_keys - 1;

    // write leaf
    leaf_c = (page_t *)leaf;
    file_write_page(leaf_pagenum, leaf_c);

	//printf("end remove_entry_from_leaf\n");

    return 0;
}

int adjust_root_leaf(pagenum_t root_pagenum) {
	//printf("start adjust_root_leaf\n");

	// read root_page
	page_t * root_c = (page_t *)malloc(sizeof(page_t));
	file_read_page(root_pagenum, root_c);
	leaf_page_t * root = (leaf_page_t *)root_c;

    if (root->num_keys == 0){

	    file_free_page(root_pagenum);

	    page_t * header_c = (page_t *)malloc(sizeof(page_t));
	    file_read_page(0, header_c);
	    header_page_t * header = (header_page_t *)header_c;

	    header->root_start = 0;
	    header->num_pages = header->num_pages - 1;

	    header_c = (page_t *)header;
	    file_write_page(0, header_c);
    }
	//printf("end adjust_root_leaf\n");
    
    return 0;
}


int delete_entry_internal(pagenum_t this_pagenum, int64_t key){
	
	// read this internal page
	page_t * this_page_c = (page_t*)malloc(sizeof(page_t));
	file_read_page(this_pagenum, this_page_c);
	internal_page_t * this_page = (internal_page_t *)this_page_c;

	int root_check = 0;
    root_check = remove_entry_from_internal(this_pagenum, key);
    if(root_check == 1)
	    return 0;

    /* Case:  deletion from a page_t below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of page_t,
     * to be preserved after deletion.
     */

    /* Case:  page_t stays at or above minimum.
     * (The simple case.)
     */
	// read this 
	file_read_page(this_pagenum, this_page_c);
	this_page = (internal_page_t *)this_page_c;

    if (this_page->num_keys > 0)
    	    free(this_page_c);
	    return 0;

    /* Case:  all keys are deleted.
     * merge is needed.
     */

	    if(this_page->num_keys == 0 && this_page->one_more_pagenum == 0){
	    	    this_pagenum = this_page->parent_pagenum;
		    delete_entry_internal(this_pagenum, key);
	    }
	    if(this_page->num_keys == 0 && this_page->one_more_pagenum != 0){
		    // redistribute
	    }

    free(this_page_c);
    
    return 0;
}

int remove_entry_from_internal(pagenum_t this_pagenum, int64_t key){

	//printf("start remove_entry_from_internal\n");
	// read internal 
	page_t * internal_c = (page_t *)malloc(sizeof(page_t));
	file_read_page(this_pagenum, internal_c);
	internal_page_t * internal = (internal_page_t *)internal_c;
	// Remove the key & value and shift other keys & values accordingly.
        int i = 0;
        pagenum_t temp_pagenum;
        while (internal->key_pagenum[i].key != key){
		i++;
	        if(internal->num_keys == 1 && internal->one_more_pagenum == 0){
			temp_pagenum = internal->key_pagenum[i].pagenum;
		}
	}
    	for (++i; i < internal->num_keys; i++){
        	internal->key_pagenum[i - 1].key = internal->key_pagenum[i].key;
        	internal->key_pagenum[i - 1].pagenum = internal->key_pagenum[i].pagenum;
	}

    	// erase last key and value for tidyness
    	internal->key_pagenum[i - 1].key = 0;
    	internal->key_pagenum[i - 1].pagenum = 0;

    	// One key fewer.
    	internal->num_keys = internal->num_keys - 1;

    	// write internal 
    	internal_c = (page_t *)internal;
    	file_write_page(this_pagenum, internal_c);

    	page_t * header_c = (page_t *)malloc(sizeof(page_t));
    	file_read_page(0, header_c);
    	header_page_t * header = (header_page_t *)header_c;
    	if(header->root_start == this_pagenum){ 
		internal_page_t * root = internal;
		if(root->num_keys == 0 && root->one_more_pagenum != 0){
			temp_pagenum = root->one_more_pagenum;
		}
		if(root->num_keys == 0){
			file_free_page(this_pagenum);
			
			header->root_start = temp_pagenum;
	    		header_c = (page_t *)header;
	    		file_write_page(0, header_c);

			return 1;
		}
	}
}
    /*


    
    // write internal 
    internal_c = (page_t *)internal;
    file_write_page(this_pagenum, internal_c);

    printf("end remove_entry_from_leaf\n");

    return 0;
}

    if(key < internal->key_pagenum[i].key){
	    file_free_page(this_pagenum);
	    internal->one_more_pagenum = 0;
	    internal_c = (page_t *)internal;
	    file_write_page(this_pagenum, internal_c);

	    free(internal_c);
	    return 0;
    }
    */


/* Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each page_t and the '|' symbol
 * to separate page_ts.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 */
/*
void print_tree() {

    page_t * p = (page_t *)malloc(sizeof(page_t));
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    // read header
    page_t * header_c = (page_t *)malloc(sizeof(page_t));
    file_read_page(0, header_c);
    header_page_t * header = (header_page_t *)header_c;

    if (header->root_start == 0) {
        printf("Empty tree.\n");
        return;
    }
    int64_t queue[4000];
    queue[0] = 0;
    enqueue(header->root_start);
    while( queue[0] != 0 ) {
	    pagenum_t p_pagenum = dequeue();
	    file_read_page(p_pagenum, p);
	    internal_page_t * p_internal = (internal_page_t *)p;
        if (p->parent != 0 && p == p->parent->pointers[0]) {
            new_rank = path_to_root( root, p );
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        for (i = 0; i < p->num_keys; i++) {
            printf("%lu ", p->keys[i]);
        }
        if (!p->is_leaf)
            for (i = 0; i <= p->num_keys; i++)
                enqueue(p->pointers[i]);
        printf("| ");
    }
    printf("\n");
}

*/

/* Helper function for printing the
 * tree out.  See print_tree.
 */
/*
void enqueue(pagenum_t pagenum) {
	int i = 0;
	while(queue[i] != 0){
		i++;
		if(i == 4000){
			printf("enque error\n");
			break;
		}

	}
	queue[i] = pagenum;
}


*/
/* Helper function for printing the
 * tree out.  See print_tree.
 */
/*
int64_t dequeue() {
	int64_t rtn = queue[0];
	int i = 1;
	while(queue[i] != 0){
		queue[i - 1] = queue[i];
		if(i == 4000)
			break;
	}

    return ;
}

*/

/* Utility function to give the length in edges
 * of the path from any page_t to the root.
 */
/*
int path_to_root( page_t * root, page_t * child ) {
    int length = 0;
    page_t * c = child;
    while (c != root) {
        c = c->parent;
        length++;
    }
    return length;
}
*/
