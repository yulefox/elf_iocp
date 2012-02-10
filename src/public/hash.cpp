/*
 * Copyright (C) 2010 - 2011 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include "hash.h"
#include <stdlib.h>
#include <string.h>
//#include "algorithm.h"
#include <assert.h>
#include "pool/pool.h"

/**
 * rehash when table gets this x full.
 */
#define ELF_HASH_FULL_TABLE         2

/**
 * self explenatory.
 */
#define ELF_HASH_DEFAULT_SIZE       10

/**
 * grow table by this factor.
 */
#define ELF_HASH_GRAW_TABLE         4

/**
 * hash value modulos table size.
 */
#define hindex(h, n)            ((h) % (n))

/**
 * @struct hash_t
 * Define the hash table.
 *
 * @li @a elem_size size of elems.
 * @li @a table_size size of tables on the chain.
 * @li @a tables talbe chain.
 * @li @a ops table operations.
 */
struct hash_t {
    size_t elem_size;
    size_t table_size;
    struct hash_iterator_t **tables;
    const struct hash_ops_t *ops;
};

/**
 * @struct hash_iterator_t
 * Define the generic iterator.
 * @param prev should not be used to index the previous iterator.
 * @param next should not be used to index the next iterator.
 */
struct hash_iterator_t {
    union {
        int n;
        void *pt;
    } key;
    void *data;
    int hkey;
    struct hash_iterator_t *prev;
    struct hash_iterator_t *next;
};

static int hash_rehash(struct hash_t *t);
static void hash_clear(struct hash_t *t);
static struct hash_iterator_t *hash_iter_find(struct hash_t *t,
    const void *key);
static struct hash_iterator_t *hash_iter_find_n(struct hash_t *t, int key);
static struct hash_iterator_t *hash_iter_insert(struct hash_t *t,
    const void *key);
static struct hash_iterator_t *hash_iter_insert_n(struct hash_t *t, int key);

static unsigned int hash_raw(const void *val);
static int cmp_raw(const void *val, const void *var);

static const struct hash_ops_t default_ops = {
    0, 0, 0, 0, 0, 0
};

struct hash_t *
hash_create(size_t size, const struct hash_ops_t *ops)
{
    struct hash_t *t;

    t = (struct hash_t *)pool_alloc(sizeof(*t));
    if (size == 0) size = ELF_HASH_DEFAULT_SIZE;
    t->tables = (struct hash_iterator_t **)pool_alloc(sizeof(*(t->tables)) *
        size);
    memset(t->tables, 0, sizeof(*(t->tables)) * size);
    t->elem_size = 0;
    t->table_size = size;
    t->ops = (ops != NULL) ? ops : &default_ops;
    return t;
}

int
hash_destroy(struct hash_t *t)
{
    if (t != NULL) {
        hash_clear(t);
        pool_free(t);
    }
    return 0;
}

size_t
hash_len(struct hash_t *t)
{
    assert(t);
    return t->elem_size;
}

void *
hash_find(struct hash_t *t, const void *key)
{
    struct hash_iterator_t *itr;

    assert(t);
    itr = hash_iter_find(t, key);
    return (itr != NULL) ? itr->data : NULL;
}

void *
hash_find_n(struct hash_t *t, int key)
{
    struct hash_iterator_t *itr;

    assert(t && key > 0);
    itr = hash_iter_find_n(t, key);
    return (itr != NULL) ? itr->data : NULL;
}

int
hash_insert(struct hash_t *t, const void *key, void *data)
{
    struct hash_iterator_t *itr;

    assert(t);
    if ((itr = hash_iter_insert(t, key)) == NULL) {
        return -1;
    }

    assert(itr);
    if (t->ops->data_free != NULL) {
        t->ops->data_free(itr->data);
    }
    if (t->ops->data_dup != NULL) {
        itr->data = t->ops->data_dup(data);
    } else {
        itr->data = data;
    }
    return 0;
}

int
hash_insert_n(struct hash_t *t, int key, void *data)
{
    struct hash_iterator_t *itr;

    assert(t);
    if ((itr = hash_iter_insert_n(t, key)) == NULL) {
        return -1;
    }

    assert(itr);
    if (t->ops->data_free != NULL) {
        t->ops->data_free(itr->data);
    }
    if (t->ops->data_dup != NULL) {
        itr->data = t->ops->data_dup(data);
    } else {
        itr->data = data;
    }
    return 0;
}

int
hash_remove(struct hash_t *t, const void *key)
{
    struct hash_iterator_t *itr, *next = NULL;

    assert(t);
    if ((itr = hash_iter_find(t, key)) == NULL) {
        return -1;
    }
    return hash_remove_i(t, itr);
}

int
hash_remove_n(struct hash_t *t, int key)
{
    struct hash_iterator_t *itr, *next = NULL;

    assert(t);
    if ((itr = hash_iter_find_n(t, key)) == NULL) {
        return -1;
    }

    if (t->ops->data_free != NULL) {
        t->ops->data_free(itr->data);
    }
    if (itr->next != NULL) {
        itr->next->prev = itr->prev;
    }
    if (itr->prev != NULL) {
        itr->prev->next = itr->next;
    } else {
        size_t i = hindex(itr->hkey, t->table_size);
        t->tables[i] = itr->next;
    }
    t->elem_size--;
    pool_free(itr);
    return 0;
}

int
hash_remove_i(struct hash_t *t, struct hash_iterator_t *itr)
{
    assert(t && itr);
    if (t->ops->key_free != NULL) {
        t->ops->key_free(itr->key.pt);
    }
    if (t->ops->data_free != NULL) {
        t->ops->data_free(itr->data);
    }
    if (itr->next != NULL) {
        itr->next->prev = itr->prev;
    }
    if (itr->prev != NULL) {
        itr->prev->next = itr->next;
    } else {
        size_t i = hindex(itr->hkey, t->table_size);
        t->tables[i] = itr->next;
    }
    t->elem_size--;
    pool_free(itr);
    return 0;
}

int
hash_begin(struct hash_t *t, struct hash_iterator_t **itr)
{
    size_t i;
    int res = -1;

    assert(t);
    *itr = NULL;
    for (i = 0; i< t->table_size; ++i) {
        if ((*itr = t->tables[i]) != NULL) {
            res = 0;
            break;
        }
    }
    return res;
}

int
hash_next(struct hash_t *t, struct hash_iterator_t **itr)
{
    size_t i;
    int res = -1;

    assert(t && itr && *itr);
    if ((*itr)->next != NULL) {
        *itr = (*itr)->next;
        res = 0;
    } else {
        for (i = (*itr)->hkey + 1; i < t->table_size; ++i) {
            if ((*itr = t->tables[i]) != NULL) {
                res = 0;
                break;
            }
        }
    }
    if (res != 0) {
        *itr = NULL;
    }
    return res;
}

int
hash_prev(struct hash_t *t, struct hash_iterator_t **itr)
{
    int i;
    int res = -1;

    assert(t && itr && *itr);
    if ((*itr)->prev != NULL) {
        *itr = (*itr)->prev;
        res = 0;
    } else {
        for (i = (int)((*itr)->hkey) - 1; i >= 0; --i) {
            if ((*itr = t->tables[i]) != NULL) {
                while ((*itr)->next != NULL) {
                    *itr = (*itr)->next;
                }
                res = 0;
                break;
            }
        }
    }
    if (res != 0) {
        *itr = NULL;
    }
    return res;
}

int
hash_key(struct hash_iterator_t *itr, void **key)
{
    assert(itr);
    *key = itr->key.pt;
    return 0;
}

int
hash_key_n(struct hash_iterator_t *itr, int *key)
{
    assert(itr);
    *key = itr->key.n;
    return 0;
}

int
hash_val(struct hash_iterator_t *itr, void **data)
{
    assert(itr);
    *data = itr->data;
    return 0;
}

int
hash_rehash(struct hash_t *t)
{
    register size_t i, h, old_size, new_size;
    register struct hash_iterator_t **tables;
    register struct hash_iterator_t *p, *q;

    assert(t);
    old_size = t->table_size;
    if (t->elem_size < old_size * ELF_HASH_FULL_TABLE) {
        return -1;
    }

    new_size = old_size * ELF_HASH_GRAW_TABLE;
    tables = (struct hash_iterator_t **)pool_alloc(sizeof(*tables) *
        new_size);
    memset(tables, 0, sizeof(*tables) * new_size);
    for (i = 0; i < old_size; ++i) {
        for (p = t->tables[i]; p != NULL; p = q) {
            q = p->next;
            h = hindex(p->hkey, new_size);
            p->next = tables[h];
            tables[h] = p;
            if (p->next != NULL)
                p->next->prev = p;
            p->prev = 0;
        }
    }
    pool_free(t->tables);
    t->tables = tables;
    t->table_size = new_size;
    return 0;
}

void
hash_clear(struct hash_t *t)
{
    size_t i;
    struct hash_iterator_t *p;
    struct hash_iterator_t *q;

    assert(t);
    for (i = 0; i< t->table_size; ++i) {
        for (p = t->tables[i]; p != NULL; p = q) {
            q = p->next;
            if (t->ops->data_free != NULL)
                t->ops->data_free(p->data);
            if (t->ops->key_free != NULL)
                t->ops->key_free(p->key.pt);
            pool_free(p);
        }
    }
    pool_free(t->tables);
}

struct hash_iterator_t *
hash_iter_find(struct hash_t *t, const void *key)
{
    unsigned int h;
    struct hash_iterator_t *itr;

    assert(t && key);
    h = hindex(t->ops->hash(key), t->table_size);
    for (itr = t->tables[h]; itr != NULL; itr = itr->next) {
        if (t->ops->compare(key, itr->key.pt) == 0) {
            return itr;
        }
    }
    return NULL;
}

struct hash_iterator_t *
hash_iter_find_n(struct hash_t *t, int key)
{
    unsigned int h;
    struct hash_iterator_t *itr;

    assert(t);
    h = hindex(key, (int)(t->table_size));
    for (itr = t->tables[h]; itr != NULL; itr = itr->next) {
        if (key == itr->key.n) {
            return itr;
        }
    }
    return NULL;
}

struct hash_iterator_t *
hash_iter_insert(struct hash_t *t, const void *key)
{
    size_t h;
    struct hash_iterator_t *itr;

    assert(t && key);
    if ((itr = hash_iter_find(t, key)) != NULL) {
        return itr;
    }

    hash_rehash(t);
    itr = (struct hash_iterator_t *)pool_alloc(sizeof(*itr));
    itr->data = NULL;
    itr->prev = NULL;
    if (t->ops->key_dup != NULL) {
        itr->key.pt = t->ops->key_dup(key);
    } else {
        itr->key.pt = (void *)key;
    }
    itr->hkey = t->ops->hash(key);
    h = hindex(itr->hkey, t->table_size);
    itr->next = t->tables[h];
    t->tables[h] = itr;
    if (itr->next != NULL)
        itr->next->prev = itr;

    t->elem_size++;
    return itr;
}

struct hash_iterator_t *
hash_iter_insert_n(struct hash_t *t, int key)
{
    size_t h;
    struct hash_iterator_t *itr;

    assert(t);
    if ((itr = hash_iter_find_n(t, key)) != NULL) {
        return itr;
    }

    hash_rehash(t);
    itr = (struct hash_iterator_t *)pool_alloc(sizeof(*itr));
    itr->data = NULL;
    itr->prev = NULL;
    itr->key.n = itr->hkey = key;
    h = hindex(itr->hkey, t->table_size);
    itr->next = t->tables[h];
    t->tables[h] = itr;
    if (itr->next != NULL)
        itr->next->prev = itr;

    t->elem_size++;
    return itr;
}
