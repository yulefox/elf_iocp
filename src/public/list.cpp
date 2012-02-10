/*
 * Copyright (C) 2011 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include "list.h"
#include <stdlib.h>
#include <assert.h>
#include "pool/pool.h"

/**
 * @struct list_t
 */
struct list_t {
    size_t size;
    struct list_iterator_t *head;
};

struct list_iterator_t {
    void *data;
    struct list_iterator_t *prev;
    struct list_iterator_t *next;
};

static int list_remove(struct list_t *l, struct list_iterator_t *n);

int
list_create(struct list_t **l)
{
    *l = (struct list_t *)pool_alloc(sizeof(**l));
    (*l)->size = 0;
    (*l)->head = NULL;
    return 0;
}

int
list_destroy(struct list_t *l)
{
    if (l != NULL) {
        struct list_iterator_t *c, *n;

        for (c = l->head; l->size-- > 0; c = n) {
            n = c->next;
            pool_free(c);
        }
        pool_free(l);
    }
    return 0;
}

size_t
list_size(struct list_t *l)
{
    assert(l);
    return l->size;
}

bool
list_empty(struct list_t *l)
{
    assert(l);
    return (l->size == 0);
}

int
list_push_back(struct list_t *l, void *val)
{
    struct list_iterator_t *n;

    assert(l);
    n = (struct list_iterator_t *)pool_alloc(sizeof(*n));
    n->data = val;
    if (l->head == NULL) {
        l->head = n;
    } else {
        n->prev = l->head->prev;
        l->head->prev->next = n;
    }
    l->head->prev = n;
    n->next = l->head;
    ++l->size;
    return 0;
}

int
list_push_front(struct list_t *l, void *val)
{
    list_push_back(l, val);
    l->head = l->head->prev;
    return 0;
}

int
list_pop_back(struct list_t *l, void **val)
{
    int res = 0;
    struct list_iterator_t *n;

    assert(l);
    if (list_empty(l)) {
        res = -1;
        *val = NULL;
    } else {
        n = l->head->prev;
        *val = n->data;
        res = list_remove(l, n);
    }
    return res;
}

int
list_pop_front(struct list_t *l, void **val)
{
    int res = 0;
    struct list_iterator_t *n;

    assert(l);
    if (list_empty(l)) {
        res = -1;
        *val = NULL;
    } else {
        n = l->head;
        *val = n->data;
        res = list_remove(l, n);
    }
    return res;
}

int
list_traverse(struct list_t *l, traverse_func func)
{
    size_t i;
    struct list_iterator_t *n;

    assert(l);
    for (i = 0, n = l->head; i++ < l->size; n = n->next) {
        func(n->data);
    }
    return 0;
}

int
list_remove(struct list_t *l, struct list_iterator_t *n)
{
    if (--l->size > 0) {
        if (n == l->head) {
            l->head = n->next;
        }
        n->prev->next = n->next;
        n->next->prev = n->prev;
    } else {
        l->head = NULL;
    }
    pool_free(n);
    return 0;
}
