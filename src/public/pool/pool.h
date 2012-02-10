/**
 * @file pool.h
 * @date 2011-09-02
 * @author Fox (yulefox at gmail.com)
 * 基于原 DataBlock 实现的内存池, 用 C 接口封装内存块的分配与释放.
 * @todo 预分配策略.
 * @todo 目前实现为多线程, 效率略低, 可考虑实现单线程内存池, 并提高通用性.
 */

#ifndef ELF_POOL_POOL_H
#define ELF_POOL_POOL_H

#pragma once

#include "public/def.h"

BEGIN_C_DECLS

/**
 * @return 0
 */
int pool_init(void);

/**
 * @return 0
 */
int pool_fini(void);
void *pool_alloc(size_t size);
void pool_free(void *block);

END_C_DECLS

#endif /* !ELF_POOL_POOL_H */