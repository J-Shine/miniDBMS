#include<stdlib.h>
#include<stdio.h>
#include<trx.h>
#include<bptree.h>

int g_trx_id = 1;
trx_t * trx_head = NULL;
trx_t * trx_tail = NULL;

void* start_pthread(){
}

int trx_make_id(){

    pthread_t new_id = g_trx_id++;
    pthread_create(&new_id, 0, start_pthread, NULL);

    /* case: none of trx exists in trx_list */
    if(trx_head == NULL && trx_tail == NULL){
        // make and initialize new trx
        trx_tail = (trx_t *)malloc(sizeof(trx_t));
        trx_tail->trx_id = new_id;
        trx_tail->lock_head = NULL;
        trx_tail->upd_hist_head = NULL;
        trx_tail->upd_hist_tail = NULL;

        // put trx into trx_list
        trx_head = trx_tail;
        trx_tail->trx_next = NULL;
        trx_head->trx_next = NULL;
        
        return trx_tail->trx_id;
    }

    /* case: trx exists in trx_list */
    else {
        // make and initialize new trx
        trx_tail->trx_next = (trx_t *)malloc(sizeof(trx_t));
        trx_tail->trx_next->trx_id = new_id;
        trx_tail->trx_next->lock_head = NULL;
        trx_tail->trx_next->upd_hist_head = NULL;
        trx_tail->trx_next->upd_hist_tail = NULL;

        // put trx into trx_list
        trx_tail = trx_tail->trx_next;
        trx_tail->trx_next = NULL;

        return trx_tail->trx_id;
    }

    /* should not get here */
    return 0;
}

int trx_clean(int trx_id, int clean_mode){

        /* find matching trx */
        trx_t * trx_iter_prev = NULL;
        trx_t * trx_iter = trx_head;
        bool trx_found = false;
        while(trx_iter != NULL && trx_found == false){
            if(trx_iter->trx_id == trx_id){
                trx_found = true;
            }
            else{
                trx_iter_prev = trx_iter;
                trx_iter = trx_iter->trx_next;
            } 
        }

        /* if clean_mode is -1, undo all modified records by trx */
        if(clean_mode == -1){
            hist_t * hist_iter = trx_iter->upd_hist_tail;
            while(hist_iter != NULL){
                find(hist_iter->table_id, hist_iter->key, hist_iter->before, trx_id, -1);
                hist_iter = hist_iter->prev;
            }
        }

        /* release all acquired lock objects */ 
        lock_t * lock_iter = trx_iter->lock_head;
        while(lock_iter != NULL){
            lock_t * lock_temp = lock_iter->trx_next_lock;

            /* release lock */
            lock_release(lock_iter);

            lock_iter = lock_temp;
        }

        /* remove the trx table entry */
        if(trx_iter_prev != NULL && trx_iter != NULL){
            trx_iter_prev->trx_next = trx_iter->trx_next;
            free(trx_iter);
        }

        /* exit the thread */
        void * ret;
	    pthread_exit(ret);

        return 0;
}
