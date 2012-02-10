#include "public/public.h"
#include "public/hash.h"

int main(void)
{
    struct hash_t *t;
    struct hash_iterator_t *itr;
    int i;

    public_init();
    t = hash_create(0, NULL);
    
    for (i = 0; i < 100000; ++i) {
        hash_insert_n(t, i, NULL);
    }

    hash_begin(t, &itr);
    hash_prev(t, &itr);
    //hash_next(t, &itr);
    hash_find_n(t, 10);
    hash_destroy(t);
    public_fini();
    return 0;
}
