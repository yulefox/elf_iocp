/**
 * @file atomic_ptr.h
 * @date 2011-09-14
 * @author Fox (yulefox at gmail.com)
 * ·ÂÐ´ ZMQ Ô­×ÓÖ¸Õë.
 */

#ifndef PUBLIC_ATOMIC_H
#define PUBLIC_ATOMIC_H

#pragma once

#include "def.h"

BEGIN_C_DECLS

int atomic_inc(volatile int *n);
int atomic_dec(volatile int *n);
int atomic_add(volatile int *n, int val);
int atomic_sub(volatile int *n, int val);
int atomic_xchg(volatile int *dst, volatile int src);

/**
 * Atomically exchanges a pail of pointers.
 * @return the initial pointer of the destination.
 */
void *atomic_ptr_xchg(void **dst, void *src);

/**
 * If dst is equal to cmp, the pointer will be set to src.
 * @return the initial pointer of the destination.
 */
void *atomic_ptr_cas(void **dst, void *src, void *cmp);

END_C_DECLS

#endif /* !PUBLIC_ATOMIC_H */
