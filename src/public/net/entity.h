/**
 * @file entity.h
 * @date 2011-09-22
 * @author Fox (yulefox at gmail.com)
 */

#ifndef NET_ENTITY_H
#define NET_ENTITY_H

#pragma once

#include "public/public.h"
#include <winsock2.h>

BEGIN_C_DECLS

enum entity_state {
    ENT_INIT,
    ENT_WAIT,
    ENT_RUN,
    ENT_EXIT,
};

int net_entity_init(void);
int net_entity_fini(void);

/**
 * @see net_load()
 */
int net_entity_create(int id, short port, const char *ip);

/*
 * Take care of thread security.
 */
int net_entity_start(int id);


int net_entity_socket(int id);

/**
 * Only called on disassociated with IOCP.
 */
int net_restart(int id);

/**
 * Send message to peer(s) with given type and id.
 * @todo peer(s) can be a set of entities of same type.
 */
int net_send(int sock, struct message_t *msg);

END_C_DECLS

#endif /* !NET_ENTITY_H */
