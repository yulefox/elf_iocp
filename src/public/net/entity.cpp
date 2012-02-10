#include "entity.h"

#include "error.h"
#include "iocp.h"

#include "public/net.h"
#include "public/hash.h"
#include "public/atomic.h"
#include "public/thread/mutex.h"
#include "public/thread/thread.h"

#define DATA_BUFSIZE 4096

#define END_POINT(addr) \
    (((u64)((addr).sin_addr.s_addr) << 32) | ((addr).sin_port))

#define SPLIT_IP(pt) ((uint)((pt) >> 32))

#define SPLIT_PORT(pt) ((ushort)((pt) & 0xFFFF))

/**
* Network context entity: Identify a server/client.
*/
struct entity_t {
    volatile entity_state state;
    int id; /* set by configuration or user program */
    SOCKET sock;
    SOCKADDR_IN addr;
    struct thread_t *th; /* for CLIENT: reconnect; for SERVER: listen */
};

/* accessed only in main thread */
static struct hash_t *entities;
static struct iocp_t *iocp;

static int net_entity_destroy(struct entity_t *ent);

/* non-block */
static unsigned int __stdcall net_accept(void *args);
static unsigned int __stdcall net_onrestart(void *args);

int
net_entity_init(void)
{
    static const struct hash_ops_t hash_ops = {
        0, 0, 0, 0, 0,
        (release_func)net_entity_destroy
    };

    MODULE_IMPORT_SWITCH;
    entities = hash_create(0, &hash_ops);
    net_iocp_create(&iocp);
    return 0;
}

int
net_entity_fini(void)
{
    MODULE_IMPORT_SWITCH;
    hash_destroy(entities);
    entities = NULL;
    net_iocp_destroy(iocp);
    return 0;
}

int
net_entity_create(int id, short port, const char *ip)
{
    int res = 0;
    struct entity_t *ent;

    ent = (struct entity_t *)hash_find_n(entities, id);
    if (ent == NULL) {
        ent = (struct entity_t *)pool_alloc(sizeof(*ent));
        ent->id = id;
        ent->sock = INVALID_SOCKET;
        ent->addr.sin_family = AF_INET;
        ent->addr.sin_addr.s_addr = (ip[0]) ? inet_addr(ip) : INADDR_ANY;
        ent->addr.sin_port = htons(port);
        ent->state = ENT_INIT;
        ent->th = NULL;
        hash_insert_n(entities, id, ent);
    } else {
        LOG_WARN("pub.net", "Entity [%d]%s:%u exists already.",
                 id, ip, port);
        res = NET_ENTITYEXIST;
    }
    return res;
}

int
net_entity_start(int id)
{
    int res;
    struct entity_t *ent;

    ent = (struct entity_t *)hash_find_n(entities, id);
    if (ent != NULL) {
        net_socket(&(ent->sock));
        if (id & NET_CLIENT_FLAG) {
            if ((res = net_connect(ent->sock, id, &(ent->addr))) == 0) {
                atomic_xchg((volatile int *) & (ent->state), ENT_RUN);
                if ((res = net_iocp_associate(iocp, ent->sock, ent->id)) == 0) {
                    thread_exit(ent->th);
                } else {
                    net_close(&(ent->sock));
                }
            }
        } else {
            if ((res = net_listen(ent->sock, &(ent->addr))) == 0) {
                atomic_xchg((volatile int *) & (ent->state), ENT_RUN);
                thread_create(&(ent->th), net_accept, ent);
                net_callback(ent->sock, NIT_LSTN, 0, NULL);
            }
        }
    } else {
        LOG_ERROR("pub.net", "Entity %d is not found.", id);
        res = NET_ENTITYNOTFOUND;
    }
    return res;
}

int
net_entity_socket(int id)
{
    struct entity_t *ent;

    ent = (struct entity_t *)hash_find_n(entities, id);
    assert(ent);
    return (int)(ent->sock);
}

int
net_restart(int id)
{
    int *pid;
    struct entity_t *ent;

    if (entities == NULL) {
        return 0;
    }

    ent = (struct entity_t *)hash_find_n(entities, id);
    assert(ent);
    switch (ent->state) {
    case ENT_RUN:
        atomic_xchg((volatile int *) & (ent->state), ENT_WAIT);
        net_close(&(ent->sock));
        thread_destroy(ent->th);
        pid = (int *)pool_alloc(sizeof(*pid));
        *pid = id;
        thread_create(&(ent->th), net_onrestart, pid);
        LOG_INFO("test.net", "%s:%u closed, restarting ...",
                 inet_ntoa(ent->addr.sin_addr),
                 ntohs(ent->addr.sin_port));
        break;
    case ENT_INIT:
        atomic_xchg((volatile int *) & (ent->state), ENT_WAIT);
        pid = (int *)pool_alloc(sizeof(*pid));
        *pid = id;
        thread_create(&(ent->th), net_onrestart, pid);
        LOG_INFO("test.net", "%s:%u closed, restarting ...",
                 inet_ntoa(ent->addr.sin_addr),
                 ntohs(ent->addr.sin_port));
        break;
    default:
        break;
    }
    return 0;
}

int
net_send(int sock, struct message_t *msg)
{
    int res;

    res = net_iocp_send(iocp, (SOCKET)sock, msg);
    net_callback(sock, NIT_SEND, res, msg);
    return res;
}

int
net_entity_destroy(struct entity_t *ent)
{
    if (ent != NULL) {
        atomic_xchg((volatile int *) & (ent->state), ENT_EXIT);
        if (ent->sock != INVALID_SOCKET) {
            net_shutdown(ent->sock);
            /** @todo wait for a moment */
            net_close(&(ent->sock));
        }
        thread_destroy(ent->th);
        pool_free(ent);
    }
    return 0;
}

unsigned int __stdcall
net_accept(void *args)
{
    SOCKET sock;
    struct entity_t *ent;
    SOCKADDR_IN addr;
    int addr_len;
    int err;
    int res;

    ent = (struct entity_t *)thread_args(args);
    addr_len = (int)sizeof(addr);
    assert(ent);
    while (!thread_isexit(args)) {
        sock = WSAAccept(ent->sock,
                         (struct sockaddr *)&addr, &addr_len, NULL, NULL);
        if (sock == INVALID_SOCKET) {
            err = WSAGetLastError();
            if (err == WSAEINTR) {
                LOG_TRACE("pub.net", "Accept thread exits.");
            } else {
                LOG_ERROR("pub.net", "WSAAccept failed: %d %s.",
                          err, net_errstr(err));
            }
            continue;
        }
        if ((res = net_iocp_associate(iocp, sock, ent->id)) == 0) {
            net_callback(sock, NIT_ACPT, 0, &(ent->id));
        }
    }
    return 0;
}

unsigned int __stdcall
net_onrestart(void *args)
{
    int res = NET_UNKNOWN;
    int *pid;

    pid = (int *)thread_args(args);
    while (!thread_isexit(args)) {
        Sleep(1000);
        net_entity_start(*pid);
    }
    pool_free(pid);
    return 0;
}
