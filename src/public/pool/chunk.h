/**
 * @file chunk.h
 * @date 2011-09-09
 * @author Fox (yulefox at gmail.com)
 * 内存块实现, 使用双向队列. chunk 使用是非线程安全的, 必须保证 chunk 在同一时
 * 刻不会被多线程访问.
 * chunk 本身不提供数据块大小记录, 但允许使用者改写数据头 (不超过初始大小), 由
 * 使用者自行裁决.
 */

#ifndef ELF_IO_CHUNKIO_H
#define ELF_IO_CHUNKIO_H

#pragma once

#include "public/def.h"

BEGIN_C_DECLS

struct chunk_t *chunk_create(size_t init_size);
void chunk_destroy(struct chunk_t *c);

/**
 * @return current writen size.
 */
int chunk_append(struct chunk_t *c, void *buf, size_t size);
int chunk_retrive(struct chunk_t *c, void *buf, size_t size);

int chunk_set(struct chunk_t *c, void *buf, size_t size);
int chunk_get(struct chunk_t *c, void *buf, size_t size);

/**
 * WSASend used.
 */
int chunk_buffers(struct chunk_t *c, struct _WSABUF **bufs, size_t *count);

END_C_DECLS

#endif /* !ELF_IO_CHUNKIO_H */