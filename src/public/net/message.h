/**
 * @file message.h
 * @authtor Fox (yulefox at gmail.com)
 * @date 2011-09-11
 * 用 C 重写消息结构, 将消息结构与网络部分剥离, 该消息在多线程中仅有一份实例,
 * No-copy
 * Dependency:
 * - memory pool(not really) of `public`.
 *
 * @todo wrap with protobuf.
 */

#ifndef NET_MESSAGE_H
#define NET_MESSAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "public/public.h"

BEGIN_C_DECLS

/**
 * for sending message: should NOT be -1, for receiving: MUST be -1.
 */
struct message_t *net_message_create(int type);
int net_message_destroy(struct message_t *msg);
bool net_message_complete(struct message_t *msg);

/**
 * Fill in message header when sending, Add header after receiving ONLY ONCE.
 */
int net_message_read(struct message_t *msg);

/**
 * Fill in message header before sending.
 */
int net_message_write(struct message_t *msg);
int net_message_get_size(struct message_t *msg, size_t *size);
int net_message_get_type(struct message_t *msg, int *type);
int net_message_set_type(struct message_t *msg, int type);
int net_message_get_ref(struct message_t *msg, size_t num);
int net_message_buffers(struct message_t *msg, struct _WSABUF **bufs,
                        size_t *count);
int net_message_append_int(struct message_t *msg, int val);
int net_message_append_int64(struct message_t *msg, __int64 val);
int net_message_append_size(struct message_t *msg, size_t val);
int net_message_append_float(struct message_t *msg, float val);
int net_message_append_str(struct message_t *msg, const char *val);
size_t net_message_append_raw(struct message_t *msg, void *val, size_t size);

/**
 * Only for IOCP receiving.
 */
size_t net_message_append_buf(struct message_t *msg, char **buf, size_t *size,
                              size_t *pos);

int net_message_retrive_int(struct message_t *msg);
__int64 net_message_retrive_int64(struct message_t *msg);
size_t net_message_retrive_size(struct message_t *msg);
float net_message_retrive_float(struct message_t *msg);
const char *net_message_retrive_str(struct message_t *msg, char *val);
int net_message_retrive_raw(struct message_t *msg, void *buf, size_t size);

END_C_DECLS

#endif /* !NET_MESSAGE_H */
