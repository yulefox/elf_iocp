/**
 * @file error.h
 * @date 2011-10-28
 * @author Fox (yulefox at gmail.com)
 * Error types.
 */

#ifndef NET_ERROR_H
#define NET_ERROR_H

#pragma once

BEGIN_C_DECLS

enum error_type {
    NET_WSAFAILED = 20000L, /* invalid socket */
    NET_SOCKETINVALID, /* invalid socket */
    NET_SOCKETNOTBOUND, /* bind failed */
    NET_SOCKETNOTCONNECTED, /* connect failed */
    NET_SOCKETNOTSHUTDOWN, /* shutdown failed */
    NET_SOCKETNOTCLOSED, /* close failed */
    NET_ENTITYEXIST, /* entity exists already */
    NET_ENTITYNOTFOUND, /* entity not found */
    NET_CONTEXTNOTFOUND, /* context not found */
    NET_IOCPNOTCREATED, /* create IOCP failed */
    NET_IOCPNOTASSOCIATED, /* associate IOCP failed */
    NET_IOCPSENDPENDING, /* maximum pending send */
    NET_IOCPNOTPOST, /* post failed */
    NET_IOCPNOTSENT, /* send failed */
    NET_IOCPNOTRECV, /* receive failed */
    NET_UNKNOWN, /* unknown error */
};

END_C_DECLS

#endif /* !NET_ERROR_H */
