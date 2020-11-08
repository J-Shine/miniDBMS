#include <lock_table.h>

struct lock_t {
	/* NO PAIN, NO GAIN. */
};

typedef struct lock_t lock_t;

int
init_lock_table()
{
	/* DO IMPLEMENT YOUR ART !!!!! */

	return 0;
}

lock_t*
lock_acquire(int table_id, int64_t key)
{
	/* ENJOY CODING !!!! */

	return (void*) 0;
}

int
lock_release(lock_t* lock_obj)
{
	/* GOOD LUCK !!! */

	return 0;
}
