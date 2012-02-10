/**
 * Copyright (C) 2010 - 2011 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file list.h
 * @author Fox (yulefox at gmail.com)
 * @date 2010-12-21
 *
 * @brief Generic list implemented by using linking list.
 *
 * Use list_iterator_t for traversing a list. If the list has only one
 * element, the previous is itself, and the next is NULL.
 */

#ifndef PUBLIC_LIST_H
#define PUBLIC_LIST_H

#include "def.h"
#include "def/type.h"

BEGIN_C_DECLS

int list_create(struct list_t **l);

int list_destroy(struct list_t *l);

size_t list_size(struct list_t *l);

bool list_empty(struct list_t *l);

/**
 * Push in the list tail.
 */
int list_push_back(struct list_t *l, void *val);

/**
 * Push in the list head.
 */
int list_push_front(struct list_t *l, void *val);

/**
 * Pop from the list tail.
 */
int list_pop_back(struct list_t *l, void **val);

/**
 * Pop from the list head.
 */
int list_pop_front(struct list_t *l, void **val);

int list_traverse(struct list_t *l, traverse_func func);

END_C_DECLS

#endif /* !PUBLIC_LIST_H */
