#include "chunk.h"
#include <assert.h>
#include <Winsock2.h>
#include "pool.h"

#define CHUNK_INIT_SIZE 64

struct chunk_t {
    size_t count; /* bulb count */
    size_t pos_wr; /* pointer to writen position of bulb_wr */
    size_t pos_rd; /* pointer to read position of bulb_rd */
    struct bulb_t *head; /* pointer to head bulb */
    struct bulb_t *bulb_wr; /* pointer to current writen bulb */
    struct bulb_t *bulb_rd; /* pointer to current read bulb */
};

/* 与 WSABUF 兼容 */
struct bulb_t {
    size_t size; /* WSABUFF 此为 u_long 类型 */
    void *buf;
    struct bulb_t *prev; /* previous of the head is a tail */
    struct bulb_t *next;
};

/** return pointer to the new bulb */
static struct bulb_t *chunk_bulb_alloc(struct bulb_t *b, size_t init_size);

static void chunk_bulb_free(struct bulb_t *b);

struct chunk_t *
    chunk_create(size_t init_size)
{
    struct chunk_t *c;

    c = (struct chunk_t *)pool_alloc(sizeof(*c));
    c->pos_wr = c->pos_rd = 0;
    c->count = 1;
    c->head = c->bulb_wr = c->bulb_rd =
        chunk_bulb_alloc(NULL, init_size ? init_size : CHUNK_INIT_SIZE);
    return c;
}

void
chunk_destroy(struct chunk_t *c)
{
    struct bulb_t *b, *p;

    assert(c);
    b = c->head;
    while (b != NULL) {
        p = b;
        b = b->next;
        chunk_bulb_free(p);
    }
    pool_free(c);
}

int
chunk_append(struct chunk_t *c, void *buf, size_t size)
{
    register struct bulb_t *cb; /* curent writen bulb */
    register size_t cp, cr, cw; /* curent position, remain size, writen size */
    register size_t rem; /* remain size */

    assert(c && buf);
    cb = c->bulb_wr;
    cp = c->pos_wr;
    cr = cb->size - cp;
    rem = size;
    cw = min(cr, rem);
    memcpy((char *)(cb->buf) + cp, buf, cw);
    if (cr >= rem) { /* enough */
        c->pos_wr += cw;
    } else {
        while ((rem -= cw) > 0) {
            buf = (char *)buf + cw;
            cb = chunk_bulb_alloc(cb, 0);
            ++(c->count);
            cr = cb->size;
            cw = min(cr, rem);
            memcpy(cb->buf, buf, cw);
        }
        c->pos_wr = cw;
        c->bulb_wr = cb;
    }
    return 0;
}

int
chunk_retrive(struct chunk_t *c, void *buf, size_t size)
{
    register struct bulb_t *cb; /* curent read bulb */
    register size_t cp, cr, cd; /* curent position, remain size, read size */
    register size_t rem; /* remain size */

    assert(c && buf);
    cb = c->bulb_rd;
    if (cb == NULL) return -1;
    cp = c->pos_rd;
    cr = cb->size - cp;
    rem = size;
    cd = min(cr, rem);
    memcpy(buf, (char *)(cb->buf) + cp, cd);
    if (cr > rem) { /* enough */
        c->pos_rd += cd;
    } else {
        while ((rem -= cd) > 0) {
            buf = (char *)buf + cd;
            cb = cb->next;
            if (cb == NULL) return -1;
            cr = cb->size;
            cd = min(cr, rem);
            memcpy(buf, cb->buf, cd);
        }
        c->pos_rd = cd;
        c->bulb_rd = cb;
    }
    return 0;
}

int
chunk_set(struct chunk_t *c, void *buf, size_t size)
{
    register struct bulb_t *cb;

    assert(c && buf);
    cb = c->head;
    assert(size < cb->size);
    memcpy(cb->buf, buf, size);
    return 0;
}

int
chunk_get(struct chunk_t *c, void *buf, size_t size)
{
    register struct bulb_t *cb;

    assert(c && buf);
    cb = c->head;
    assert(size < cb->size);
    memcpy(buf, cb->buf, size);
    return 0;
}

int 
chunk_buffers(struct chunk_t *c, struct _WSABUF **bufs, size_t *count)
{
    size_t i = 0;
    struct bulb_t *b;

    assert(c);
    b = c->head;
    *count = c->count;
    *bufs = (struct _WSABUF *)pool_alloc(sizeof(**bufs) * (*count));
    while (b != NULL) {
        ((*bufs)[i]).buf = (char *)(b->buf);
        ((*bufs)[i]).len = (u_long)(b->size);
        b = b->next;
        ++i;
    }
    ((*bufs)[i - 1]).len = (u_long)(c->pos_wr);
    return 0;
}

struct bulb_t *
    chunk_bulb_alloc(struct bulb_t *b, size_t size)
{
    struct bulb_t *n;

    n = (struct bulb_t *)pool_alloc(sizeof(*n));
    n->next = NULL;

    if (b == NULL) {
        n->size = size;
    } else {
        b->next = n;
        n->size = b->size << 1;
    }
    n->prev = b;
    n->buf = pool_alloc(n->size);
    return n;
}

void
chunk_bulb_free(struct bulb_t *b)
{
    assert(b);
    pool_free(b->buf);
    pool_free(b);
}
