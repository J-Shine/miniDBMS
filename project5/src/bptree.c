#include "bptree.h"
#include <lock_table.h>
#include <trx.h>

/* Finds and returns the record to which
 * a key refers.
 * lock_mode: 0 is Shared mode
 *            1 is Exclusive mode
 *            -1 is Undo mode(doesn't acquire new lock)
 */
int find(int table_id, int64_t key, char * ret, int trx_id, int lock_mode) {
    /*
    printf("start find()\n");
	printf("table_id: %d, key: %ld\n", table_id, key);
    */
    int i = 0;
    pagenum_t * pagenum = (pagenum_t *)malloc(sizeof(pagenum_t));
    leaf_page_t * leaf = (leaf_page_t *)malloc(sizeof(leaf_page_t));
    int c = find_leaf(table_id, key, leaf, pagenum);
    if (c == -1) {
	    free(pagenum);
	    free(leaf);
        //printf("end find()1\n");
	    return -1;
    }
    for (i = 0; i < leaf->num_keys; i++)
        if (leaf->key_value[i].key == key) {
            if(0 <= lock_mode)
                lock_acquire(table_id, key, trx_id, lock_mode);
            break;
        }
    if (i == leaf->num_keys) {
	    /////////////////////////
    	    unpin(table_id, *pagenum);
	    free(pagenum);
	    free(leaf);
        //printf("end find()2\n");
            return -1;
    }

    else if(lock_mode == 0){
	    strncpy(ret, leaf->key_value[i].value, sizeof(leaf->key_value[i].value));
	    //////////////////////////
    	unpin(table_id, *pagenum);
	    free(pagenum);
	    free(leaf);
        //printf("end find()3\n");
        return 0;
    }
    else if(lock_mode == 1){
        /* find matching trx */
        trx_t * trx_iter = trx_head;
        bool trx_found = false;
        while(trx_iter != NULL && trx_found == false){
            if(trx_iter->trx_id == trx_id){
                trx_found = true;
            }
            else{
                trx_iter = trx_iter->trx_next;
            }
        }

        /* save history */
        /* create and init hist object */
        hist_t * new_hist = (hist_t *)malloc(sizeof(hist_t));
        new_hist->table_id = table_id;
        new_hist->key = key;
        new_hist->before = (char *)malloc(sizeof(char) * 120);
        new_hist->after = (char *)malloc(sizeof(char) * 120);
        strncpy(new_hist->before, leaf->key_value[i].value, sizeof(leaf->key_value[i].value));
        strncpy(new_hist->after, ret, sizeof(ret));

        /* connect object to the history linked list */
        /* case no element exists */
        if(trx_iter->upd_hist_tail == NULL){
            new_hist->prev = NULL;
            new_hist->next = NULL;
            trx_iter->upd_hist_tail = new_hist;
            trx_iter->upd_hist_head = new_hist;
        }
        /* case one or more element exists */
        else if(trx_iter->upd_hist_tail != NULL){
            new_hist->prev = trx_iter->upd_hist_tail;
            new_hist->next = NULL;
            trx_iter->upd_hist_tail = new_hist;
            new_hist->prev->next = new_hist;
        }

        /* update the record */
	    strncpy(leaf->key_value[i].value, ret, sizeof(ret));

    	//////////////////////////
    	unpin(table_id, *pagenum);

	    free(pagenum);
	    free(leaf);
        //printf("end find()4\n");
        return 0;
    }
    /////////////////////////// unpin leaf from find_leaf
    unpin(table_id, *pagenum);
    free(pagenum);
    free(leaf);
    //printf("end find()5\n");
    return 0;
}

int find_leaf(int table_id, int64_t key, leaf_page_t * leaf_page, pagenum_t * pagenum) {
    //printf("start find_leaf()\n");

    // get header page
    page_t * header_c = (page_t *)malloc(sizeof(page_t));
    /////////////////////////////////////////////////////////////
    buffer_read_buffer(table_id, 0, header_c);
    header_page_t * header_page = (header_page_t *)header_c;
    pagenum_t root_start = header_page->root_start;
    unpin(table_id, 0);
    //printf("checking header_page\n");
    // check header page
    /*
    printf("%ld\n", header_page->free_start);
    printf("%ld\n", header_page->root_start);
    printf("%ld\n", header_page->num_pages);
    */

    if (root_start == 0){
	    /*
	    printf("header_page->root_start %ld\n", header_page->root_start);
	    printf("header_page->root_start %ld\n", header_page->free_start);
	    printf("header_page->root_start %ld\n", header_page->num_pages);
	    */
        //printf("end find_leaf()1\n");
        return -1; 
    }
    *pagenum = root_start;
    page_t * internal_c = (page_t *)malloc(sizeof(page_t));
    /////////////////////////////////////////////////////////////
    buffer_read_buffer(table_id, root_start, internal_c);
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
	    if(i == 0){
		    /////////////////////////
		    unpin(table_id, *pagenum);
    		    //printf("u6\n");
		    *pagenum = internal_page->one_more_pagenum;
	    }
	    else{
		    /////////////////////////
		    unpin(table_id, *pagenum);
    		    //printf("u7\n");
		    *pagenum = internal_page->key_pagenum[i - 1].pagenum;
	    }
	    
	    /////////////////////////////////////////////////////////////
	    buffer_read_buffer(table_id, *pagenum, internal_c);
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
    //printf("end find_leaf()2\n");
    return 0;
}

/* Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each page_t and the '|' symbol
 * to separate page_ts.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 */
void print_tree(int table_id) {

    page_t * c = (page_t *)malloc(sizeof(page_t));
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    // read header
    page_t * header_c = (page_t *)malloc(sizeof(page_t));
    /////////////////////////////////////////////////////
    buffer_read_buffer(table_id, 0, header_c);
    header_page_t * header = (header_page_t *)header_c;
    pagenum_t root_start = header->root_start;
    ////////////////////////////////////////
    unpin(table_id, 0);
    //printf("u8\n");

    if (header->root_start == 0) {
        printf("Empty tree.\n");
        return;
    }
    leaf_page_t * c_leaf = NULL;
    int64_t queue[4000];
    int head = 0;
    int tail = 0;
    queue[tail] = 0;
    enqueue(root_start, queue, &head, &tail);
    while( head != tail ) {
	    pagenum_t c_pagenum = dequeue(queue, &head, &tail);
    	    /////////////////////////////////////////////////////
	    buffer_read_buffer(table_id, c_pagenum, c);
	    internal_page_t * c_internal = (internal_page_t *)c;
	    ////////////////////////////////////////////////////
	    unpin(table_id, c_pagenum);
    	    //printf("u9\n");
        if (c_internal->parent_pagenum != 0) {
            new_rank = path_to_root(table_id, root_start, c_pagenum);
            if (rank != new_rank) {
                rank = new_rank;
                printf("\n");
            }
        }
	if(c_internal->is_leaf == 0){
        	for (i = 0; i < c_internal->num_keys; i++)
			printf("%ld ", c_internal->key_pagenum[i].key);
        }
	else{
		c_leaf = (leaf_page_t *)c_internal;
		for (i = 0; i < c_leaf->num_keys; i++)
			printf("%ld ", c_leaf->key_value[i].key);
	}
        if (!c_internal->is_leaf){
		if(c_internal->one_more_pagenum != 0)
			enqueue(c_internal->one_more_pagenum, queue, &head, &tail);
		for (i = 0; i < c_internal->num_keys; i++)
			enqueue(c_internal->key_pagenum[i].pagenum, queue, &head, &tail);
	}
        printf("| ");
    }
    printf("\n");

    free(c);
    free(header_c);
    return;
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
void enqueue(pagenum_t pagenum, pagenum_t * queue, int * head, int * tail) {
	queue[*tail] = pagenum;
	(*tail)++;
	if(*tail >= 4000)
		*tail = 0;
	if(*head == *tail)
		printf("queue is fully occupied\n");
}

/* Helper function for printing the
 * tree out.  See print_tree.
 */
pagenum_t dequeue(pagenum_t * queue, int * head, int * tail) {
	pagenum_t rtn = queue[*head];
	(*head)++;
	if(*head >= 4000)
		*head = 0;
	return rtn;
}


/* Utility function to give the length in edges
 * of the path from any page_t to the root.
 */
int path_to_root(int table_id, pagenum_t root_pagenum, pagenum_t child_pagenum) {
    int length = 0;
    if(root_pagenum == child_pagenum){
	    return length;
    }
    page_t * internal_c = (page_t *)malloc(sizeof(page_t));
    ///////////////////////////////////////////////////////
    buffer_read_buffer(table_id, child_pagenum, internal_c);
    internal_page_t * internal = (internal_page_t *)internal_c;

    pagenum_t parent_pagenum = internal->parent_pagenum;
    length++;
    ///////////////////////////////////////////////////////
    unpin(table_id, child_pagenum);
    //printf("u10\n");
    
    while (parent_pagenum != root_pagenum) {
	    page_t * p = (page_t *)malloc(sizeof(page_t));
	    //////////////////////////////////////////////
	    buffer_read_buffer(table_id, parent_pagenum, p);
	    internal_page_t * parent = (internal_page_t *)p;
	    //////////////////////////////////////////////
	    unpin(table_id, parent_pagenum);
    	    //printf("u11\n");
            parent_pagenum = parent->parent_pagenum;
            length++;
	    

	    free(p);
    }
    free(internal);
    return length;
}

// Insertion

int insert(int table_id, int64_t key, char * value, int trx_id, int lock_mode) {
	page_t * c1 = (page_t *)malloc(sizeof(page_t));
	//////////////////////////////////////////////
	buffer_read_buffer(table_id, 0, c1);
	header_page_t * head = (header_page_t *)c1;
	pagenum_t root_start = head->root_start;
	///////////////////
	unpin(table_id, 0);
    	    //printf("u12\n");

    /* The current implementation ignores
     * duplicates.
     */

    if (find(table_id, key, value, trx_id, lock_mode) == 0){
	    free(c1);
	    return -1;
    }
    //printf("finish find() in the insert()\n");

    /* Case: the tree does not exist yet.
     * Start a new tree.
     */

    //printf("root_start %ld\n", root_start);
    if (root_start == 0){ 
    		//printf("ii1\n");

        if((start_new_tree(table_id, key, value)) < 0){
    		//printf("ii2\n");
	    	free(c1);
		return -1;
	}
	for(int j = 0; j < num_buffer; j++){
		if(buffer_pool[j]->table_id != -1){
			for(int i = 0; i < 10; i++){
				//printf("after start_new_tree buffer_pool1[%d] %ld\n", j, buffer_pool[j]->page.buffer[i]);
			}
		}
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
    /////////////////////////////////////////read leaf from find_leaf
    find_leaf(table_id, key, leaf, pagenum);


    /* Case: leaf has room for key and pointer.
     */
    // record가 29개 이하면 넣고 끝낸다.
    if (leaf->num_keys < LEAF_ORDER - 2) {
	    //printf("nnn: %d\n", leaf->num_keys);
	    ////////////////////////// unpin leaf from find_leaf
	    unpin(table_id, *pagenum);
    	    //printf("u13\n");
        if((insert_into_leaf(table_id, leaf, key, value, pagenum)) < 0){
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
    ////////////////////////// unpin leaf from find_leaf
    unpin(table_id, *pagenum);
    	    //printf("u14\n");
    insert_into_leaf_after_splitting(table_id, leaf, key, value, leaf->parent_pagenum, *pagenum);
    //printf("after the insert_into_leaf_after_splitting\n");
    free(leaf);
    free(pagenum);
    return 0;
}


/* First insertion:
 * start a new tree.
 */
int start_new_tree(int table_id, int64_t key, char * value) {

        // get new root num
        pagenum_t new_root_pagenum = file_alloc_page_buffer(table_id);
    	set_free_pages_buffer(table_id);
	// read header page
	page_t * c = (page_t *)malloc(sizeof(page_t));
	//////////////////////////////////////////////read header
	buffer_read_buffer(table_id, 0, c);
	header_page_t * header = (header_page_t *)c;
	header->root_start = new_root_pagenum;


    page_t * c_leaf = (page_t *)malloc(sizeof(page_t));
    ///////////////////////////////////////////////read root
    buffer_read_buffer(table_id, header->root_start, c_leaf);
    /*
    for(int i = 0; i < 10; i++){
	    printf("c_leafs2 %ld\n",c_leaf->buffer[i]);
    }
    */
    leaf_page_t * leaf_page = (leaf_page_t *)malloc(sizeof(page_t));
    leaf_page = (leaf_page_t *)c_leaf;

    /*
	for(int j = 0; j < num_buffer; j++){
		if(buffer_pool[j]->table_id != -1){
			for(int i = 0; i < 10; i++){
				printf("after start_new_tree buffer_pool2[%d] %ld\n", j, buffer_pool[j]->page.buffer[i]);
			}
		}
	}
    */

    /*
    for(int i = 0; i < 10; i++){
	    printf("c_leafs2.4 %ld\n",c_leaf->buffer[i]);
    }
    */
    // set leaf_page
    leaf_page->parent_pagenum = 0;
    leaf_page->is_leaf = 1;
    leaf_page->num_keys = 1;
    leaf_page->right_sib_pagenum = 0;
    /*
    for(int i = 0; i < 10; i++){
	    printf("c_leafs2.5 %ld\n",c_leaf->buffer[i]);
    }
    */
    
    // save record to leaf
    leaf_page->key_value[0].key = key;
    strncpy(leaf_page->key_value[0].value, value, sizeof(value));

    // write root_page to disk
    c_leaf = (page_t *)leaf_page;
    ///////////////////////////////////////////////write root
    /*
    for(int i = 0; i < 10; i++){
	    printf("c_leafs3 %ld\n",c_leaf->buffer[i]);
    }
    */
    buffer_write_buffer(table_id, header->root_start, c_leaf);
    ///////////////////////////////////////////////unpin root
    unpin(table_id, header->root_start);
    /*
    printf("u15\n");
	for(int j = 0; j < num_buffer; j++){
		if(buffer_pool[j]->table_id != -1){
			for(int i = 0; i < 10; i++){
				printf("after start_new_tree buffer_pool3[%d] %ld\n", j, buffer_pool[j]->page.buffer[i]);
			}
		}
	}
    */
    
    // write header_page to disk
    page_t * c_header = (page_t *)header;
    ///////////////////////////////////////////////write header
    buffer_write_buffer(table_id, 0, c_header);
    ///////////////////////////////////////////////unpin header
    unpin(table_id, 0);
    //printf("u16\n");

    free(leaf_page);

    return 0;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
int insert_into_leaf(int table_id, leaf_page_t * leaf, int64_t key, char * value, pagenum_t * pagenum) {

    int i, insertion_point;

    //////////////////////// pin leaf
    pin(table_id, *pagenum);

    insertion_point = 0;
    while (insertion_point < leaf->num_keys && leaf->key_value[insertion_point].key <= key){
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

    buffer_write_buffer(table_id, *pagenum, c);
    //////////////////////// unpin leaf
    unpin(table_id, *pagenum);
    //printf("u17\n");

    return 0;
}

/* Inserts a new key and a new value 
 * into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
int insert_into_leaf_after_splitting(int table_id, leaf_page_t * leaf, int64_t key, char * value, pagenum_t parent_pagenum, pagenum_t pagenum) {

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
    make_leaf(table_id, new_leaf, parent_pagenum, new_pagenum);
    //////////////////////////////////pin pagenum, *new_pagenum
    pin(table_id, pagenum);
    pin(table_id, *new_pagenum);

    // move records to temp
    insertion_index = LEAF_ORDER - 2;
    //printf("leaf->num_keys %d\n", leaf->num_keys);

    for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
        if (j == insertion_index) j++;
        temp_keys[j] = leaf->key_value[i].key;
        strncpy(temp_values[j], leaf->key_value[i].value, 120);
    }

    // insert new record to temp
    temp_keys[insertion_index] = key;
    strncpy(temp_values[insertion_index], value, 120);

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
    //printf("outside the make_leaf5\n");

    char zeros[120] = { 0 };
    for (i = leaf->num_keys; i < LEAF_ORDER - 1; i++){
        leaf->key_value[i].key = 0;
    }

    //printf("outside the make_leaf6\n");
    // set right sib
    leaf->right_sib_pagenum = *new_pagenum; 
    new_leaf->right_sib_pagenum = 0;
    //printf("outside the make_leaf7\n");

    // write leaf
    page_t * leaf_c = (page_t *)leaf;
    /////////////////////////////////
    buffer_write_buffer(table_id, pagenum, leaf_c);
    //printf("outside the make_leaf8\n");

    // write new_leaf
    page_t * new_leaf_c = (page_t *)new_leaf;
    /////////////////////////////////
    buffer_write_buffer(table_id, *new_pagenum, new_leaf_c);
    //printf("outside the make_leaf9\n");


    // insert key into parent
    new_key = new_leaf->key_value[0].key;
    //printf("outside the make_leaf10\n");

    //////////////////////// unpin pagenum, *new_pagenum 
    unpin(table_id, pagenum);
    //printf("u18\n");
    unpin(table_id, *new_pagenum);
    //printf("u19\n");

    insert_into_parent(table_id, leaf_c, new_key, new_leaf_c, parent_pagenum, pagenum, *new_pagenum);

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
int make_leaf(int table_id, leaf_page_t * leaf, pagenum_t parent_pagenum, pagenum_t * new_pagenum) {
	// get new page number
	*new_pagenum = file_alloc_page_buffer(table_id);
	set_free_pages_buffer(table_id);
	//printf("*new_pagenum %ld\n", *new_pagenum);
	
	// read *new_pagenum to leaf
	page_t * leaf_c = (page_t *)leaf;
	buffer_read_buffer(table_id, *new_pagenum, leaf_c); 
	leaf = (leaf_page_t *)leaf_c;

	// set leaf
	leaf->parent_pagenum = parent_pagenum;
	leaf->is_leaf = 1;
	leaf->num_keys = 0;

	// write leaf to the disk
	leaf_c = (page_t *)leaf;
	//printf("*new_pagenum %ld\n", *new_pagenum);
	buffer_write_buffer(table_id, *new_pagenum, leaf_c);
	unpin(table_id, *new_pagenum);
	//printf("u20\n");

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
int insert_into_parent(int table_id, page_t * left, int64_t key, page_t * right, pagenum_t this_pagenum, pagenum_t left_pagenum, pagenum_t right_pagenum) {

    /* Case: new root. */

	// read left and right again
	buffer_read_buffer(table_id, left_pagenum, left);
	buffer_read_buffer(table_id, right_pagenum, right);
	//printf("inside the insert_into_parent\n");

    /*
	for(int i = 0; i < num_buffer; i++){
		printf("buffer_pool[%d]->table_id %d\n", i, buffer_pool[i]->table_id);
		printf("buffer_pool[%d]->page_num %ld\n", i, buffer_pool[i]->page_num);
		for(int j = 0; j < 10; j++){
			printf("buffer_pool[%d]->page.buffer[%d] %ld\n", i, j, buffer_pool[i]->page.buffer[j]);
		}
	}
    */

	unpin(table_id, left_pagenum);
	unpin(table_id, right_pagenum);

    if (this_pagenum == 0){
        insert_into_new_root(table_id, left, key, right, left_pagenum, right_pagenum);
    	return 0;
    }

    /* Case: leaf or page_t. (Remainder of
     * function body.)  
     */

    /* Simple case: the new key fits into the page_t. 
     */
    page_t * this_page_c = (page_t *)malloc(sizeof(page_t));
    //////////////////////////////////////////////////////// read this_pagenum
    buffer_read_buffer(table_id, this_pagenum, this_page_c); 
    internal_page_t * this_page = (internal_page_t *)this_page_c;

    if (this_page->num_keys < INTERNAL_ORDER - 1){
	////////////////////////////// unpin this_pagenum
	unpin(table_id, this_pagenum);
	//printf("u21\n");
        insert_into_internal(table_id, this_page, key, right_pagenum, this_pagenum);
	return 0;
    }


    /* Harder case:  split a page_t in order 
     * to preserve the B+ tree properties.
     */

    pagenum_t parent_pagenum = this_page->parent_pagenum;

    ////////////////////////////// unpin this_pagenum
    unpin(table_id, this_pagenum);
    //printf("u22\n");

    insert_into_internal_after_splitting(table_id, this_page, key, right_pagenum, parent_pagenum, this_pagenum);
    free(this_page_c);


    return 0;
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
int insert_into_new_root(int table_id, page_t * left, int64_t key, page_t * right, pagenum_t left_pagenum, pagenum_t right_pagenum) {
	int k;
    /*
	for(k = 0; k < num_buffer; k++){
		if(buffer_pool[k]->table_id != -1){
			for(int i = 0; i < 10; i++){
				printf("after insert_into_new_root buffer_pool1[%d] %ld\n", k, buffer_pool[k]->page.buffer[i]);
			}
		}
	}
    */

	page_t * new_root_internal_c = (page_t *)malloc(sizeof(page_t));
	internal_page_t * new_root_internal = (internal_page_t *)new_root_internal_c;
	pagenum_t * new_pagenum = (pagenum_t *)malloc(sizeof(pagenum_t));
	///////////////////////////////////////////////// read internal
	make_internal(table_id, new_root_internal, 0, new_pagenum);
    /*
	for(int j = 0; j < num_buffer; j++){
		if(buffer_pool[j]->table_id != -1){
			for(int i = 0; i < 10; i++){
				printf("after insert_into_new_root buffer_pool1[%d] %ld\n", j, buffer_pool[j]->page.buffer[i]);
			}
		}
	}
    */
	pin(table_id, *new_pagenum);
	new_root_internal->key_pagenum[0].key = key;
	new_root_internal->key_pagenum[0].pagenum = right_pagenum;
	new_root_internal->one_more_pagenum = left_pagenum;
	new_root_internal->num_keys += 1;
    /*
	for(int j = 0; j < num_buffer; j++){
		if(buffer_pool[j]->table_id != -1){
			for(int i = 0; i < 10; i++){
				printf("after insert_into_new_root buffer_pool1.5[%d] %ld\n", j, buffer_pool[j]->page.buffer[i]);
			}
		}
	}
    */
	
	// write new root to buffer 
	page_t * new_root_c = (page_t *)new_root_internal;
	/////////////////////////////////////////////////////// write internal
	buffer_write_buffer(table_id, *new_pagenum, new_root_c);
	////////////////////////////// unpin internal
	unpin(table_id, *new_pagenum);
	//printf("u23\n");
    /*
	for(int j = 0; j < num_buffer; j++){
		if(buffer_pool[j]->table_id != -1){
			for(int i = 0; i < 10; i++){
				printf("after insert_into_new_root buffer_pool2[%d] %ld\n", j, buffer_pool[j]->page.buffer[i]);
			}
		}
	}
    */
	
	// write new root pagenum to header
	page_t * header_c = (page_t *)malloc(sizeof(page_t));
	////////////////////////////////////////////// read header
	buffer_read_buffer(table_id, 0, header_c);
	header_page_t * header = (header_page_t *)header_c;
        header->root_start = *new_pagenum;	
	header_c = (page_t *)header;
	////////////////////////////////////////////// write header and unpin
	buffer_write_buffer(table_id, 0, header_c);
	unpin(table_id, 0);
	//printf("u24\n");
    /*
	for(int j = 0; j < num_buffer; j++){
		if(buffer_pool[j]->table_id != -1){
			for(int i = 0; i < 10; i++){
				printf("after insert_into_new_root buffer_pool3[%d] %ld\n", j, buffer_pool[j]->page.buffer[i]);
			}
		}
	}
    */
	
	// write new root as parent for left and right
	page_t * child = (page_t *)malloc(sizeof(page_t));

	//////////////////////////////////// read left_pagenum
	buffer_read_buffer(table_id, left_pagenum, child); 
	child->buffer[0] = *new_pagenum;
	//////////////////////////////////// wrtie and unpin left_pagenum
	buffer_write_buffer(table_id, left_pagenum, child);
	unpin(table_id, left_pagenum);
	//printf("u25\n");
    /*
	for(int j = 0; j < num_buffer; j++){
		if(buffer_pool[j]->table_id != -1){
			for(int i = 0; i < 10; i++){
				printf("after insert_into_new_root buffer_pool4[%d] %ld\n", j, buffer_pool[j]->page.buffer[i]);
			}
		}
	}
    */

	//////////////////////////////////// read right_pagenum
	buffer_read_buffer(table_id, right_pagenum, child); 
	child->buffer[0] = *new_pagenum;
	//////////////////////////////////// write and upin right_pagenum
	buffer_write_buffer(table_id, right_pagenum, child);
	unpin(table_id, right_pagenum);
	//printf("u26\n");


	free(new_root_internal);
	free(new_pagenum);
	free(child);
	return 0;
}


/* Creates a new leaf by creating a page_t
 * and then adapting it appropriately.
 */
int make_internal(int table_id, internal_page_t * internal, pagenum_t parent_pagenum, pagenum_t * new_pagenum) {

	// get new page number
	*new_pagenum = file_alloc_page_buffer(table_id);

	page_t * internal_c = (page_t *)internal;
	buffer_read_buffer(table_id, *new_pagenum, internal_c);
	internal = (internal_page_t*)internal_c;
	// set internal 
	internal->parent_pagenum = parent_pagenum;
	internal->is_leaf = 0;
	internal->num_keys = 0;

	// write leaf to the buffer 
	internal_c = (page_t *)internal;
	buffer_write_buffer(table_id, *new_pagenum, internal_c);
	//printf("inside make_internal\n");
	unpin(table_id, *new_pagenum);

	return 0;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
int insert_into_internal(int table_id, internal_page_t * internal, int64_t key, pagenum_t pagenum, pagenum_t parent_pagenum) {

    /////////////////////////// pin pagenum
    pin(table_id, pagenum);
    int i, insertion_point;

    // read internal again
    page_t * internal_c = (page_t *)internal;
    buffer_read_buffer(table_id, parent_pagenum, internal_c);
    internal = (internal_page_t *)internal_c; 

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
    internal_c = (page_t *)internal;

    ///////////////////////////// write pagenum
    buffer_write_buffer(table_id, parent_pagenum, internal_c);
    /////////////////////////// unpin pagenum
    unpin(table_id, parent_pagenum);
    unpin(table_id, pagenum);
    //printf("u27\n");

    return 0;
}



/* Inserts a new key and a new value 
 * into a internal so as to exceed
 * the tree's order, causing the internal to be split
 * in half.
 */
int insert_into_internal_after_splitting(int table_id, internal_page_t * internal, int64_t key, pagenum_t record_pagenum, pagenum_t parent_pagenum, pagenum_t this_pagenum) {

	//printf("inside insert_into_internal_after_splitting\n");
    /*
	for(int i = 0; i < num_buffer; i++){
		printf("buffer_pool[%d]->table_id %d\n", i, buffer_pool[i]->table_id);
		printf("buffer_pool[%d]->page_num %ld\n", i, buffer_pool[i]->page_num);
		for(int j = 0; j < 10; j++){
			printf("buffer_pool[%d]->page.buffer[%d] %ld\n", i, j, buffer_pool[i]->page.buffer[j]);
		}
	}
    */
    internal_page_t * new_internal = (internal_page_t *)malloc(sizeof(page_t));
    int64_t * temp_keys = malloc(INTERNAL_ORDER * sizeof(int64_t));
    pagenum_t * temp_pagenums = malloc(INTERNAL_ORDER * sizeof(int64_t));
    int insertion_index, split, i, j;
    int64_t new_key = 0;

    // read internal again
    page_t * internal_c = (page_t *)internal;
    //printf("read internal again\n");
    //printf("this_pagenum %ld\n", this_pagenum);
    buffer_read_buffer(table_id, this_pagenum, internal_c);
    internal = (internal_page_t *)internal_c;

    // make internal 
    //printf("before the make_internal\n");
    pagenum_t * new_pagenum = (pagenum_t *)malloc(sizeof(pagenum_t));
    make_internal(table_id, new_internal, parent_pagenum, new_pagenum);
    /////////////////////////////////// pin new_pagenum & this_pagenum
    pin(table_id, *new_pagenum);

    // move records to temp
    insertion_index = INTERNAL_ORDER - 1;

    int h = 0;
    for (i = 0, j = 0; i < internal->num_keys; i++, j++) {
        if (j == insertion_index) j++;
        temp_keys[j] = internal->key_pagenum[i].key;
        temp_pagenums[j] = internal->key_pagenum[i].pagenum;
	h++;
    }

    // insert new record to temp
    temp_keys[insertion_index] = key;
    temp_pagenums[insertion_index] = record_pagenum;
    h++;

    internal->num_keys = 0;
    split = cut(INTERNAL_ORDER - 1);
    /*
    for(int k = 0; k < h; k++){
	   printf("temp_keys: [%d], %ld\n", k, temp_keys[k]); 
	   printf("temp_pagenums: [%d], %ld\n", k, temp_pagenums[k]); 
    }
    printf("split: %d\n", split);
    */

    for (i = 0; i < split; i++) {
        internal->key_pagenum[i].key = temp_keys[i];
        internal->key_pagenum[i].pagenum = temp_pagenums[i];
        internal->num_keys = internal->num_keys + 1;
    }

    int64_t parent_inserting_key = temp_keys[i];
    new_internal->one_more_pagenum = temp_pagenums[i]; 

    page_t * child = (page_t *)malloc(sizeof(page_t));
    /////////////////////////////////////// read temp_pagenums[i]
    buffer_read_buffer(table_id, temp_pagenums[i], child);
    child->buffer[0] = *new_pagenum; 
    ////////////////////////////////////// wrtie temp_pagenums[i]
    buffer_write_buffer(table_id, temp_pagenums[i], child);
    ////////////////////////////////////// unpin temp_pagenums[i]
    unpin(table_id, temp_pagenums[i]);
    //printf("u28\n");
    free(child);
    i++;

    
    for (j = 0; i < INTERNAL_ORDER; i++, j++) {
        new_internal->key_pagenum[j].key = temp_keys[i];
        new_internal->key_pagenum[j].pagenum = temp_pagenums[i];
        new_internal->num_keys = new_internal->num_keys + 1;
	// change parent pagenum to new parent pagenum
	page_t * child = (page_t *)malloc(sizeof(page_t));
	////////////////////////////////// read temp_pagenums[i]
	buffer_read_buffer(table_id, temp_pagenums[i], child);
	child->buffer[0] = *new_pagenum; 
	////////////////////////////////// write temp_pagenums[i]
	buffer_write_buffer(table_id, temp_pagenums[i], child);
	////////////////////////////////// unpin temp_pagenums[i]
	unpin(table_id, temp_pagenums[i]);
	//printf("u29\n");
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
    //printf("internal_splitting: this_pagenum %ld\n", this_pagenum); 
    internal_c = (page_t *)internal;
    buffer_write_buffer(table_id, this_pagenum, internal_c);

    // write new_internal
    //printf("internal_splitting: *new_pagenum %ld\n", *new_pagenum); 
    page_t * new_internal_c = (page_t *)new_internal;
    buffer_write_buffer(table_id, *new_pagenum, new_internal_c);

    // insert key into parent
    parent_pagenum = internal->parent_pagenum;

    /////////////////////////////////// unpin *new_pagenum & this_pagenum
    unpin(table_id, *new_pagenum);
    unpin(table_id, this_pagenum);
    //printf("u30\n");
    //printf("before insert_into_parent\n");
    /*
	for(int i = 0; i < num_buffer; i++){

		printf("buffer_pool[%d]->table_id %d\n", i, buffer_pool[i]->table_id);
		printf("buffer_pool[%d]->page_num %ld\n", i, buffer_pool[i]->page_num);
		for(int j = 0; j < 10; j++){
			printf("buffer_pool[%d]->page.buffer[%d] %ld\n", i, j, buffer_pool[i]->page.buffer[j]);
		}
	}
    */

    insert_into_parent(table_id, internal_c, parent_inserting_key, new_internal_c, parent_pagenum, this_pagenum, *new_pagenum);

    free(temp_keys);
    free(temp_pagenums);
    free(new_pagenum);
    free(new_internal);

    return 0;

}


