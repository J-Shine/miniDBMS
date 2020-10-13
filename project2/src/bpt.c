#include "bpt.h"

// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * page_t.  Every page_t has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal page_t has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
page_t * queue = NULL;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
bool verbose_output = false;

/* Second message to the user.
 */
void usage_2( void ) {
    printf("Enter any of the following commands after the prompt > :\n"
    "\ti <k>  -- Insert <k> (an integer) as both key and value).\n"
    "\tf <k>  -- Find the value under key <k>.\n"
    "\tp <k> -- Print the path from the root to key k and its associated "
           "value.\n"
    "\tr <k1> <k2> -- Print the keys and values found in the range "
            "[<k1>, <k2>\n"
    "\td <k>  -- Delete key <k> and its associated value.\n"
    "\tx -- Destroy the whole tree.  Start again with an empty tree of the "
           "same order.\n"
    "\tt -- Print the B+ tree.\n"
    "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
    "\tv -- Toggle output of pointer addresses (\"verbose\") in tree and "
           "leaves.\n"
    "\tq -- Quit. (Or use Ctl-D.)\n"
    "\t? -- Print this help message.\n");
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
void enqueue( page_t * new_page ) {
    page_t * c;
    if (queue == NULL) {
        queue = new_page;
        queue->next = NULL;
    }
    else {
        c = queue;
        while(c->next != NULL) {
            c = c->next;
        }
        c->next = new_page;
        new_page->next = NULL;
    }
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
page_t * dequeue( void ) {
    page_t * p = queue;
    queue = queue->next;
    p->next = NULL;
    return p;
}


/* Prints the bottom row of keys
 * of the tree (with their respective
 * pointers, if the verbose_output flag is set.
 */
void print_leaves( page_t * root ) {
    int i;
    page_t * c = root;
    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    while (!c->is_leaf)
        c = c->pointers[0];
    while (true) {
        for (i = 0; i < c->num_keys; i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)c->pointers[i]);
            printf("%lu ", c->keys[i]);
        }
        if (verbose_output)
            printf("%lx ", (unsigned long)c->pointers[order - 1]);
        if (c->pointers[order - 1] != NULL) {
            printf(" | ");
            c = c->pointers[order - 1];
        }
        else
            break;
    }
    printf("\n");
}


/* Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 */
int height( page_t * root ) {
    int h = 0;
    page_t * c = root;
    while (!c->is_leaf) {
        c = c->pointers[0];
        h++;
    }
    return h;
}


/* Utility function to give the length in edges
 * of the path from any page_t to the root.
 */
int path_to_root( page_t * root, page_t * child ) {
    int length = 0;
    page_t * c = child;
    while (c != root) {
        c = c->parent;
        length++;
    }
    return length;
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
void print_tree( page_t * root ) {

    page_t * p = NULL;
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    queue = NULL;
    enqueue(root);
    while( queue != NULL ) {
        p = dequeue();
        if (p->parent != NULL && p == p->parent->pointers[0]) {
            new_rank = path_to_root( root, p );
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        if (verbose_output) 
            printf("(%lx)", (unsigned long)p);
        for (i = 0; i < p->num_keys; i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)p->pointers[i]);
            printf("%lu ", p->keys[i]);
        }
        if (!p->is_leaf)
            for (i = 0; i <= p->num_keys; i++)
                enqueue(p->pointers[i]);
        if (verbose_output) {
            if (p->is_leaf) 
                printf("%lx ", (unsigned long)p->pointers[order - 1]);
            else
                printf("%lx ", (unsigned long)p->pointers[p->num_keys]);
        }
        printf("| ");
    }
    printf("\n");
}


/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
void find_and_print(page_t * root, int64_t key, bool verbose) {
    key_value_t * r = find(root, key, verbose);
    if (r == NULL)
        printf("Record not found under key %lu.\n", key);
    else 
        printf("Record at %lx -- key %lu, value %d.\n",
                (unsigned long)r, key, r->value);
}


/* Finds and prints the keys, pointers, and values within a range
 * of keys between key_start and key_end, including both bounds.
 */
void find_and_print_range( page_t * root, uint64_t key_start, uint64_t key_end,
        bool verbose ) {
    uint64_t i;
    uint64_t array_size = key_end - key_start + 1;
    uint64_t returned_keys[array_size];
    void * returned_pointers[array_size];
    uint64_t num_found = find_range( root, key_start, key_end, verbose,
            returned_keys, returned_pointers );
    if (!num_found)
        printf("None found.\n");
    else {
        for (i = 0; i < num_found; i++)
            printf("Key: %lu   Location: %lx  Value: %d\n",
                    returned_keys[i],
                    (unsigned long)returned_pointers[i],
                    ((key_value_t *)
                     returned_pointers[i])->value);
    }
}


/* Finds keys and their pointers, if present, in the range specified
 * by key_start and key_end, inclusive.  Places these in the arrays
 * returned_keys and returned_pointers, and returns the number of
 * entries found.
 */
/*
uint64_t find_range( page_t * root, uint64_t key_start, uint64_t key_end, bool verbose,
        uint64_t returned_keys[], void * returned_pointers[]) {
    int i, num_found;
    num_found = 0;
    page_t * n = find_leaf( root, key_start, verbose );
    if (n == NULL) return 0;
    for (i = 0; i < n->num_keys && n->keys[i] < key_start; i++) ;
    if (i == n->num_keys) return 0;
    while (n != NULL) {
        for ( ; i < n->num_keys && n->keys[i] <= key_end; i++) {
            returned_keys[num_found] = n->keys[i];
            returned_pointers[num_found] = n->pointers[i];
            num_found++;
        }
        n = n->pointers[order - 1];
        i = 0;
    }
    return num_found;
}

*/

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
page_t * find_leaf(int64_t key) {
    // get header page
    header_page_t * header_page;
    pread(fd, header_page, 4096, 0);
    // check header page
    if (header_page->root_start == 0){
        return NULL;
    }
    page_t * c;
    pread(fd, c, 4096, header_page->root_start * 4096);
    internal_page_t * internal_page = (internal_page_t *)c;
    while(!internal_page->is_leaf){
	    int i = 0;
	    while(i < internal_page->num_keys){
		    if(key >= internal_page->key_pagenum[i].key) i++;
		    else break;
	    }
	    pagenum_t next_page_num = internal_page->key_pagenum[i].pagenum;
	    pread(fd, c, 4096, next_page_num * 4096);
	    internal_page = (internal_page_t *)c;
    }
    leaf_page_t * leaf_page = (leaf_page_t *)internal_page;
    return leaf_page;
}


/* Finds and returns the record to which
 * a key refers.
 */
char * find(int64_t key) {
    int i = 0;
    leaf_page_t * c = find_leaf(key);
    if (c == NULL) return NULL;
    for (i = 0; i < c->num_keys; i++)
        if (c->keys[i] == key) break;
    if (i == c->num_keys) 
        return NULL;
    else
        return c->key_value[i].value;
}

/* Finds the appropriate place to
 * split a page_t that is too big into two.
 */
int cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}


// INSERTION

/* Creates a new record to hold the value
 * to which a key refers.
 */
key_value_t * make_record(int value) {
    key_value_t * new_record = (key_value_t *)malloc(sizeof(key_value_t));
    if (new_record == NULL) {
        perror("Record creation.");
        exit(EXIT_FAILURE);
    }
    else {
        new_record->value = value;
    }
    return new_record;
}


/* Creates a new general page_t, which can be adapted
 * to serve as either a leaf or an internal page_t.
 */
page_t * make_page_t( void ) {
    page_t * new_page_t;
    new_page_t = malloc(sizeof(page_t));
    if (new_page_t == NULL) {
        perror("Node creation.");
        exit(EXIT_FAILURE);
    }
    new_page_t->keys = malloc( (order - 1) * sizeof(int) );
    if (new_page_t->keys == NULL) {
        perror("New page_t keys array.");
        exit(EXIT_FAILURE);
    }
    new_page_t->pointers = malloc( order * sizeof(void *) );
    if (new_page_t->pointers == NULL) {
        perror("New page_t pointers array.");
        exit(EXIT_FAILURE);
    }
    new_page_t->is_leaf = false;
    new_page_t->num_keys = 0;
    new_page_t->parent = NULL;
    new_page_t->next = NULL;
    return new_page_t;
}

/* Creates a new leaf by creating a page_t
 * and then adapting it appropriately.
 */
page_t * make_leaf( void ) {
    page_t * leaf = make_page_t();
    leaf->is_leaf = true;
    return leaf;
}


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the page_t to the left of the key to be inserted.
 */
int get_left_index(page_t * parent, page_t * left) {

    int left_index = 0;
    while (left_index <= parent->num_keys && 
            parent->pointers[left_index] != left)
        left_index++;
    return left_index;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
page_t * insert_into_leaf( page_t * leaf, uint64_t key, key_value_t * pointer ) {

    int i, insertion_point;

    insertion_point = 0;
    while (insertion_point < leaf->num_keys && leaf->keys[insertion_point] < key)
        insertion_point++;

    for (i = leaf->num_keys; i > insertion_point; i--) {
        leaf->keys[i] = leaf->keys[i - 1];
        leaf->pointers[i] = leaf->pointers[i - 1];
    }
    leaf->keys[insertion_point] = key;
    leaf->pointers[insertion_point] = pointer;
    leaf->num_keys++;
    return leaf;
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
page_t * insert_into_leaf_after_splitting(page_t * root, page_t * leaf, uint64_t key, key_value_t * pointer) {

    page_t * new_leaf;
    int * temp_keys;
    void ** temp_pointers;
    int insertion_index, split, new_key, i, j;

    new_leaf = make_leaf();

    temp_keys = malloc( order * sizeof(int) );
    if (temp_keys == NULL) {
        perror("Temporary keys array.");
        exit(EXIT_FAILURE);
    }

    temp_pointers = malloc( order * sizeof(void *) );
    if (temp_pointers == NULL) {
        perror("Temporary pointers array.");
        exit(EXIT_FAILURE);
    }

    insertion_index = 0;
    while (insertion_index < order - 1 && leaf->keys[insertion_index] < key)
        insertion_index++;

    for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
        if (j == insertion_index) j++;
        temp_keys[j] = leaf->keys[i];
        temp_pointers[j] = leaf->pointers[i];
    }

    temp_keys[insertion_index] = key;
    temp_pointers[insertion_index] = pointer;

    leaf->num_keys = 0;

    split = cut(order - 1);

    for (i = 0; i < split; i++) {
        leaf->pointers[i] = temp_pointers[i];
        leaf->keys[i] = temp_keys[i];
        leaf->num_keys++;
    }

    for (i = split, j = 0; i < order; i++, j++) {
        new_leaf->pointers[j] = temp_pointers[i];
        new_leaf->keys[j] = temp_keys[i];
        new_leaf->num_keys++;
    }

    free(temp_pointers);
    free(temp_keys);

    new_leaf->pointers[order - 1] = leaf->pointers[order - 1];
    leaf->pointers[order - 1] = new_leaf;

    for (i = leaf->num_keys; i < order - 1; i++)
        leaf->pointers[i] = NULL;
    for (i = new_leaf->num_keys; i < order - 1; i++)
        new_leaf->pointers[i] = NULL;

    new_leaf->parent = leaf->parent;
    new_key = new_leaf->keys[0];

    return insert_into_parent(root, leaf, new_key, new_leaf);
}


/* Inserts a new key and pointer to a page_t
 * into a page_t into which these can fit
 * without violating the B+ tree properties.
 */
page_t * insert_into_page_t(page_t * root, page_t * n, 
        int left_index, uint64_t key, page_t * right) {
    int i;

    for (i = n->num_keys; i > left_index; i--) {
        n->pointers[i + 1] = n->pointers[i];
        n->keys[i] = n->keys[i - 1];
    }
    n->pointers[left_index + 1] = right;
    n->keys[left_index] = key;
    n->num_keys++;
    return root;
}


/* Inserts a new key and pointer to a page_t
 * into a page_t, causing the page_t's size to exceed
 * the order, and causing the page_t to split into two.
 */
page_t * insert_into_page_t_after_splitting(page_t * root, page_t * old_page_t, int left_index, 
        uint64_t key, page_t * right) {

    int i, j, split, k_prime;
    page_t * new_page_t, * child;
    int * temp_keys;
    page_t ** temp_pointers;

    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new page_t and copy half of the 
     * keys and pointers to the old page_t and
     * the other half to the new.
     */

    temp_pointers = malloc( (order + 1) * sizeof(page_t *) );
    if (temp_pointers == NULL) {
        perror("Temporary pointers array for splitting page_ts.");
        exit(EXIT_FAILURE);
    }
    temp_keys = malloc( order * sizeof(int) );
    if (temp_keys == NULL) {
        perror("Temporary keys array for splitting page_ts.");
        exit(EXIT_FAILURE);
    }

    for (i = 0, j = 0; i < old_page_t->num_keys + 1; i++, j++) {
        if (j == left_index + 1) j++;
        temp_pointers[j] = old_page_t->pointers[i];
    }

    for (i = 0, j = 0; i < old_page_t->num_keys; i++, j++) {
        if (j == left_index) j++;
        temp_keys[j] = old_page_t->keys[i];
    }

    temp_pointers[left_index + 1] = right;
    temp_keys[left_index] = key;

    /* Create the new page_t and copy
     * half the keys and pointers to the
     * old and half to the new.
     */  
    split = cut(order);
    new_page_t = make_page_t();
    old_page_t->num_keys = 0;
    for (i = 0; i < split - 1; i++) {
        old_page_t->pointers[i] = temp_pointers[i];
        old_page_t->keys[i] = temp_keys[i];
        old_page_t->num_keys++;
    }
    old_page_t->pointers[i] = temp_pointers[i];
    k_prime = temp_keys[split - 1];
    for (++i, j = 0; i < order; i++, j++) {
        new_page_t->pointers[j] = temp_pointers[i];
        new_page_t->keys[j] = temp_keys[i];
        new_page_t->num_keys++;
    }
    new_page_t->pointers[j] = temp_pointers[i];
    free(temp_pointers);
    free(temp_keys);
    new_page_t->parent = old_page_t->parent;
    for (i = 0; i <= new_page_t->num_keys; i++) {
        child = new_page_t->pointers[i];
        child->parent = new_page_t;
    }

    /* Insert a new key into the parent of the two
     * page_ts resulting from the split, with
     * the old page_t to the left and the new to the right.
     */

    return insert_into_parent(root, old_page_t, k_prime, new_page_t);
}



/* Inserts a new page_t (leaf or internal page_t) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
page_t * insert_into_parent(page_t * root, page_t * left, uint64_t key, page_t * right) {

    int left_index;
    page_t * parent;

    parent = left->parent;

    /* Case: new root. */

    if (parent == NULL)
        return insert_into_new_root(left, key, right);

    /* Case: leaf or page_t. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * page_t.
     */

    left_index = get_left_index(parent, left);


    /* Simple case: the new key fits into the page_t. 
     */

    if (parent->num_keys < order - 1)
        return insert_into_page_t(root, parent, left_index, key, right);

    /* Harder case:  split a page_t in order 
     * to preserve the B+ tree properties.
     */

    return insert_into_page_t_after_splitting(root, parent, left_index, key, right);
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
page_t * insert_into_new_root(page_t * left, uint64_t key, page_t * right) {

    page_t * root = make_page_t();
    root->keys[0] = key;
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->num_keys++;
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    return root;
}



/* First insertion:
 * start a new tree.
 */
page_t * start_new_tree(uint64_t key, key_value_t * pointer) {

    page_t * root = make_leaf();
    root->keys[0] = key;
    root->pointers[0] = pointer;
    root->pointers[order - 1] = NULL;
    root->parent = NULL;
    root->num_keys++;
    return root;
}



/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
page_t * insert(page_t * root, int64_t key, char * value) {

    key_value_t * pointer;
    page_t * leaf;

    /* The current implementation ignores
     * duplicates.
     */

    if (find(root, key, false) != NULL)
        return root;

    /* Create a new record for the
     * value.
     */
    pointer = make_record(value);


    /* Case: the tree does not exist yet.
     * Start a new tree.
     */

    if (root == NULL) 
        return start_new_tree(key, pointer);


    /* Case: the tree already exists.
     * (Rest of function body.)
     */

    leaf = find_leaf(root, key, false);

    /* Case: leaf has room for key and pointer.
     */

    if (leaf->num_keys < order - 1) {
        leaf = insert_into_leaf(leaf, key, pointer);
        return root;
    }


    /* Case:  leaf must be split.
     */

    return insert_into_leaf_after_splitting(root, leaf, key, pointer);
}




// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a page_t's nearest neighbor (sibling)
 * to the left if one exists.  If not (the page_t
 * is the leftmost child), returns -1 to signify
 * this special case.
 */
int get_neighbor_index( page_t * n ) {

    int i;

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */
    for (i = 0; i <= n->parent->num_keys; i++)
        if (n->parent->pointers[i] == n)
            return i - 1;

    // Error state.
    printf("Search for nonexistent pointer to page_t in parent.\n");
    printf("Node:  %#lx\n", (unsigned long)n);
    exit(EXIT_FAILURE);
}


page_t * remove_entry_from_page_t(page_t * n, uint64_t key, page_t * pointer) {

    int i, num_pointers;

    // Remove the key and shift other keys accordingly.
    i = 0;
    while (n->keys[i] != key)
        i++;
    for (++i; i < n->num_keys; i++)
        n->keys[i - 1] = n->keys[i];

    // Remove the pointer and shift other pointers accordingly.
    // First determine number of pointers.
    num_pointers = n->is_leaf ? n->num_keys : n->num_keys + 1;
    i = 0;
    while (n->pointers[i] != pointer)
        i++;
    for (++i; i < num_pointers; i++)
        n->pointers[i - 1] = n->pointers[i];


    // One key fewer.
    n->num_keys--;

    // Set the other pointers to NULL for tidiness.
    // A leaf uses the last pointer to point to the next leaf.
    if (n->is_leaf)
        for (i = n->num_keys; i < order - 1; i++)
            n->pointers[i] = NULL;
    else
        for (i = n->num_keys + 1; i < order; i++)
            n->pointers[i] = NULL;

    return n;
}


page_t * adjust_root(page_t * root) {

    page_t * new_root;

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    if (root->num_keys > 0)
        return root;

    /* Case: empty root. 
     */

    // If it has a child, promote 
    // the first (only) child
    // as the new root.

    if (!root->is_leaf) {
        new_root = root->pointers[0];
        new_root->parent = NULL;
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.

    else
        new_root = NULL;

    free(root->keys);
    free(root->pointers);
    free(root);

    return new_root;
}


/* Coalesces a page_t that has become
 * too small after deletion
 * with a neighboring page_t that
 * can accept the additional entries
 * without exceeding the maximum.
 */
page_t * coalesce_page_ts(page_t * root, page_t * n, page_t * neighbor, int neighbor_index, int k_prime) {

    int i, j, neighbor_insertion_index, n_end;
    page_t * tmp;

    /* Swap neighbor with page_t if page_t is on the
     * extreme left and neighbor is to its right.
     */

    if (neighbor_index == -1) {
        tmp = n;
        n = neighbor;
        neighbor = tmp;
    }

    /* Starting point in the neighbor for copying
     * keys and pointers from n.
     * Recall that n and neighbor have swapped places
     * in the special case of n being a leftmost child.
     */

    neighbor_insertion_index = neighbor->num_keys;

    /* Case:  nonleaf page_t.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     */

    if (!n->is_leaf) {

        /* Append k_prime.
         */

        neighbor->keys[neighbor_insertion_index] = k_prime;
        neighbor->num_keys++;


        n_end = n->num_keys;

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            neighbor->keys[i] = n->keys[j];
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
            n->num_keys--;
        }

        /* The number of pointers is always
         * one more than the number of keys.
         */

        neighbor->pointers[i] = n->pointers[j];

        /* All children must now point up to the same parent.
         */

        for (i = 0; i < neighbor->num_keys + 1; i++) {
            tmp = (page_t *)neighbor->pointers[i];
            tmp->parent = neighbor;
        }
    }

    /* In a leaf, append the keys and pointers of
     * n to the neighbor.
     * Set the neighbor's last pointer to point to
     * what had been n's right neighbor.
     */

    else {
        for (i = neighbor_insertion_index, j = 0; j < n->num_keys; i++, j++) {
            neighbor->keys[i] = n->keys[j];
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
        }
        neighbor->pointers[order - 1] = n->pointers[order - 1];
    }

    root = delete_entry(root, n->parent, k_prime, n);
    free(n->keys);
    free(n->pointers);
    free(n); 
    return root;
}


/* Redistributes entries between two page_ts when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small page_t's entries without exceeding the
 * maximum
 */
page_t * redistribute_page_ts(page_t * root, page_t * n, page_t * neighbor, int neighbor_index, 
        int k_prime_index, int k_prime) {  

    int i;
    page_t * tmp;

    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     */

    if (neighbor_index != -1) {
        if (!n->is_leaf)
            n->pointers[n->num_keys + 1] = n->pointers[n->num_keys];
        for (i = n->num_keys; i > 0; i--) {
            n->keys[i] = n->keys[i - 1];
            n->pointers[i] = n->pointers[i - 1];
        }
        if (!n->is_leaf) {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys];
            tmp = (page_t *)n->pointers[0];
            tmp->parent = n;
            neighbor->pointers[neighbor->num_keys] = NULL;
            n->keys[0] = k_prime;
            n->parent->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
        }
        else {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
            neighbor->pointers[neighbor->num_keys - 1] = NULL;
            n->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            n->parent->keys[k_prime_index] = n->keys[0];
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     */

    else {  
        if (n->is_leaf) {
            n->keys[n->num_keys] = neighbor->keys[0];
            n->pointers[n->num_keys] = neighbor->pointers[0];
            n->parent->keys[k_prime_index] = neighbor->keys[1];
        }
        else {
            n->keys[n->num_keys] = k_prime;
            n->pointers[n->num_keys + 1] = neighbor->pointers[0];
            tmp = (page_t *)n->pointers[n->num_keys + 1];
            tmp->parent = n;
            n->parent->keys[k_prime_index] = neighbor->keys[0];
        }
        for (i = 0; i < neighbor->num_keys - 1; i++) {
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        if (!n->is_leaf)
            neighbor->pointers[i] = neighbor->pointers[i + 1];
    }

    /* n now has one more key and one more pointer;
     * the neighbor has one fewer of each.
     */

    n->num_keys++;
    neighbor->num_keys--;

    return root;
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
page_t * delete_entry( page_t * root, page_t * n, uint64_t key, void * pointer ) {

    int min_keys;
    page_t * neighbor;
    int neighbor_index;
    int k_prime_index, k_prime;
    int capacity;

    // Remove key and pointer from page_t.

    n = remove_entry_from_page_t(n, key, pointer);

    /* Case:  deletion from the root. 
     */

    if (n == root) 
        return adjust_root(root);


    /* Case:  deletion from a page_t below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of page_t,
     * to be preserved after deletion.
     */

    min_keys = n->is_leaf ? cut(order - 1) : cut(order) - 1;

    /* Case:  page_t stays at or above minimum.
     * (The simple case.)
     */

    if (n->num_keys >= min_keys)
        return root;

    /* Case:  page_t falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */

    /* Find the appropriate neighbor page_t with which
     * to coalesce.
     * Also find the key (k_prime) in the parent
     * between the pointer to page_t n and the pointer
     * to the neighbor.
     */

    neighbor_index = get_neighbor_index( n );
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    k_prime = n->parent->keys[k_prime_index];
    neighbor = neighbor_index == -1 ? n->parent->pointers[1] : 
        n->parent->pointers[neighbor_index];

    capacity = n->is_leaf ? order : order - 1;

    /* Coalescence. */

    if (neighbor->num_keys + n->num_keys < capacity)
        return coalesce_page_ts(root, n, neighbor, neighbor_index, k_prime);

    /* Redistribution. */

    else
        return redistribute_page_ts(root, n, neighbor, neighbor_index, k_prime_index, k_prime);
}



/* Master deletion function.
 */
page_t * delete(page_t * root, uint64_t key) {

    page_t * key_leaf;
    key_value_t * key_record;

    key_record = find(root, key, false);
    key_leaf = find_leaf(root, key, false);
    if (key_record != NULL && key_leaf != NULL) {
        root = delete_entry(root, key_leaf, key, key_record);
        free(key_record);
    }
    return root;
}


void destroy_tree_page_ts(page_t * root) {
    int i;
    if (root->is_leaf)
        for (i = 0; i < root->num_keys; i++)
            free(root->pointers[i]);
    else
        for (i = 0; i < root->num_keys + 1; i++)
            destroy_tree_page_ts(root->pointers[i]);
    free(root->pointers);
    free(root->keys);
    free(root);
}


page_t * destroy_tree(page_t * root) {
    destroy_tree_page_ts(root);
    return NULL;
}

