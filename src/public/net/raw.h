/**
 * @file raw.h
 * @date 2011-09-28
 * @author Fox (yulefox at gmail.com)
 */

#ifndef NET_RAW_H
#define NET_RAW_H

#pragma once

#include "public/def.h"
#include <winsock2.h>

BEGIN_C_DECLS

#define NET_CLIENT_FLAG 1

typedef int(*net_func)(SOCKET s, int type, int res, void *args);

enum io_type {
    NIT_INIT, /* initialize */
    NIT_LSTN, /* listen */
    NIT_CNCT, /* connect */
    NIT_ACPT, /* accept */
    NIT_ACLS, /* active close */
    NIT_PCLS, /* passive close */
    NIT_SEND, /* WSASend */
    NIT_SNDF, /* send completed */
    NIT_RECV, /* WSARecv */
    NIT_RCVF, /* receive completed */
};

int net_socket(SOCKET *s);
int net_listen(SOCKET s, SOCKADDR_IN *addr);
int net_connect(SOCKET s, int id, SOCKADDR_IN *addr);
int net_shutdown(SOCKET s);
int net_close(SOCKET *s);
const char *net_errstr(int err);

END_C_DECLS

#endif /* !NET_RAW_H */
