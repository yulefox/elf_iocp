/**
 * @file iocp.h
 * @date 2011-09-22
 * @author Fox (yulefox at gmail.com)
 */

#ifndef NET_IOCP_H
#define NET_IOCP_H

#pragma once

#include "public/public.h"
#include <winsock2.h>

BEGIN_C_DECLS

int net_iocp_create(struct iocp_t **iocp);
int net_iocp_destroy(struct iocp_t *iocp);
int net_iocp_context_get(struct context_t **con, struct iocp_t *iocp,
SOCKET s);
int net_iocp_set(struct iocp_t *iocp, int con_recv);
int net_iocp_associate(struct iocp_t *iocp, SOCKET s, int id);
int net_iocp_send(struct iocp_t *iocp, SOCKET s, struct message_t *msg);
int net_iocp_recv(SOCKET s);

END_C_DECLS

#endif /* !NET_IOCP_H */
