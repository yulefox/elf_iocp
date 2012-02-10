/**
 * @file xqueue.h
 * @date 2011-09-14
 * @author Fox (yulefox at gmail.com)
 * 仿写 ZMQ Lock-Free 队列, 加入 spare 结点意在减少抖动.
 */

#ifndef PUBLIC_XQUEUE_H
#define PUBLIC_XQUEUE_H

#pragma once

#include "def.h"

BEGIN_C_DECLS

/**
 * Create new queue with a fresh chunk.
 * @param chunk_num size of each chunk.
 */
struct xqueue_t *xqueue_create(size_t chunk_num);

void xqueue_destroy(struct xqueue_t *q);

/**
 * Push in the queue tail.
 */
int xqueue_push(struct xqueue_t *q, void *val);

/**
 * Pop from the queue head.
 */
int xqueue_pop(struct xqueue_t *q, void **val);

END_C_DECLS

#endif /* !PUBLIC_XQUEUE_H */
