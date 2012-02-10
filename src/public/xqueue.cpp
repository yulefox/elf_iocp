#include "xqueue.h"
#include <stdlib.h>
#include "atomic.h"
#include "pool/pool.h"

struct xqueue_t {
    struct xchunk_t *head;
    struct xchunk_t *tail;
    struct xchunk_t *spare; /* spare chunk */
    size_t head_pos;
    size_t tail_pos;
    size_t chunk_num;
    void *wr; /* point to the first item to be writen */
    void *rd; /* point to the first item to be read */
};

struct xchunk_t {
    void **values;
    xchunk_t *prev;
    xchunk_t *next;
};

static bool xqueue_check_push(struct xqueue_t *q);
static bool xqueue_check_pop(struct xqueue_t *q);

struct xqueue_t *xqueue_create(size_t chunk_num)
{
    struct xqueue_t *q;
    struct xchunk_t *c;

    q = (struct xqueue_t *)pool_alloc(sizeof(*q));
    c = (struct xchunk_t *)pool_alloc(sizeof(*c));
    q->head = q->tail = c;
    q->head_pos = q->tail_pos = 0;
    q->chunk_num = chunk_num;
    c->values = (void **)pool_alloc(sizeof(void *) * chunk_num);
    c->prev = c->next = NULL;
    q->wr = q->rd = c->values;
    q->spare = NULL;
    return q;
}

void
xqueue_destroy(struct xqueue_t *q)
{
    struct xchunk_t *c, *n, *t;

    c = q->head;
    t = q->tail;
    while (c != t) {
        n = c->next;
        pool_free(c->values);
        pool_free(c);
        c = n;
    }
    pool_free(t->values);
    pool_free(t);
    c = (struct xchunk_t *)atomic_ptr_xchg((void **)(&q->spare), NULL);
    if (c) {
        pool_free(c->values);
        pool_free(c);
    }
    pool_free(q);
}

int
xqueue_push(struct xqueue_t *q, void *val)
{
    q->tail->values[q->tail_pos] = val;
    if (++q->tail_pos == q->chunk_num) { /* full */
        register struct xchunk_t *c;

        c = (struct xchunk_t *)atomic_ptr_xchg((void **)(&q->spare), NULL);
        if (!c) { /* first time */
            c = (struct xchunk_t *)pool_alloc(sizeof(*c));
            c->values = (void **)pool_alloc(sizeof(void *) * q->chunk_num);
            c->prev = q->tail;
            c->next = NULL;
        }
        q->tail->next = c;
        q->tail = c;
        q->tail_pos = 0;
    }
    atomic_ptr_xchg(&(q->wr), q->tail->values + q->tail_pos);
    return 0;
}

int
xqueue_pop(struct xqueue_t *q, void **val)
{
    register int res = -1;

    if (xqueue_check_pop(q)) {
        *val = q->head->values[q->head_pos];
        if (++q->head_pos == q->chunk_num) {
            register struct xchunk_t *o, *c;

            o = q->head;
            q->head = o->next;
            q->head->prev = NULL;
            q->head_pos = 0;

            c = (struct xchunk_t *)atomic_ptr_xchg((void **)(&q->spare), o);
            if (c){
                pool_free(c->values);
                pool_free(c);
            }
        }
        res = 0;
    }
    q->rd = q->head->values + q->head_pos;
    return res;
}

bool
xqueue_check_push(struct xqueue_t *q)
{
    register bool res = true;

    if (!q->wr) {
        res = false;
    }
    return res;
}

bool
xqueue_check_pop(struct xqueue_t *q)
{
    register bool res = false;

    if (q->rd != q->wr) {
        res = true;
    }
    return res;
}
