#include "iocp.h"

#include "error.h"

#include "public/net.h"
#include "public/atomic.h"
#include "public/list.h"
#include "public/hash.h"
#include "public/thread/mutex.h"
#include "public/xqueue.h"

#define CHUNK_SIZE 1024
#define CHUNK_COUNT 1024
#define MAX_PENDING 10000000 // 10 MiB
#define MAX_SEQNUM 4096
#define MAX_CONTEXT_NUM 1024

typedef int (*ol_callback)(struct iocp_t *iocp, DWORD size,
                           struct context_t *con, struct overlapped_t *ol);

struct iocp_t {
    HANDLE cp; /* completed port */
    DWORD worker_num;
    HANDLE *workers; /* worker thread */
    struct hash_t *contexts;
};

struct context_t {
    int id;
    SOCKET sock;
    size_t pos_rd;
    volatile bool active;
    volatile size_t pending_send;
    struct message_t *msg;
    struct xqueue_t *send_bufs;
    struct xqueue_t *recv_msgs;
};

struct overlapped_t {
    OVERLAPPED overlapped;
    io_type type;
    size_t count;
    LPWSABUF bufs;
    struct message_t *msg;
    volatile int sn;
    ol_callback func;
};

static int net_context_create(struct context_t **con, SOCKET s, int id);
static int net_context_destroy(struct context_t *con);
static int net_context_send(struct iocp_t *iocp,
                            struct context_t *con,
                            struct message_t *msg);
static int net_iocp_disassociate(struct iocp_t *iocp, struct context_t *con);
static int net_iocp_post(struct iocp_t *iocp, DWORD size,
                         struct context_t *con, struct overlapped_t *ol);
static int net_iocp_onhtbk(struct iocp_t *iocp, DWORD size,
                           struct context_t *con, struct overlapped_t *ol);
static int net_iocp_oninit(struct iocp_t *iocp, DWORD size,
                           struct context_t *con, struct overlapped_t *ol);
static int net_iocp_onsend(struct iocp_t *iocp, DWORD size,
                           struct context_t *con, struct overlapped_t *ol);
static int net_iocp_onrecv(struct iocp_t *iocp, DWORD size,
                           struct context_t *con, struct overlapped_t *ol);
static int net_iocp_onsndf(struct iocp_t *iocp, DWORD size,
                           struct context_t *con, struct overlapped_t *ol);
static int net_iocp_onrcvf(struct iocp_t *iocp, DWORD size,
                           struct context_t *con, struct overlapped_t *ol);
static int net_overlapped_create(struct overlapped_t **ol, io_type type);
static int net_overlapped_set(struct overlapped_t *ol, io_type type);
static int net_overlapped_destroy(struct overlapped_t *ol);
unsigned int __stdcall net_iocp_worker(void *args);

int
net_iocp_create(struct iocp_t **iocp)
{
    int res = 0;
    DWORD i, cn;
    SYSTEM_INFO sysinfo;
    static const struct hash_ops_t hash_ops = {
        0, 0, 0, 0, 0,
        (release_func)net_context_destroy
    };

    *iocp = (struct iocp_t *)pool_alloc(sizeof(**iocp));
    GetSystemInfo(&sysinfo);
    cn = sysinfo.dwNumberOfProcessors;
    (*iocp)->cp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, cn);
    if ((*iocp)->cp == NULL) {
        pool_free(*iocp);
        (*iocp) = NULL;
        LOG_ERROR("pub.net", "CreateIoCompletionPort failed: %u.",
                  GetLastError());
        res = NET_IOCPNOTCREATED;
    } else {
        (*iocp)->worker_num = cn;
        (*iocp)->workers = (HANDLE *)pool_alloc(cn * sizeof(HANDLE));
        for (i = 0; i < cn; ++i) {
            (*iocp)->workers[i] = (HANDLE)_beginthreadex(NULL, 0, net_iocp_worker,
                                  *iocp, 0, 0);
        }
        (*iocp)->contexts = hash_create(MAX_CONTEXT_NUM, &hash_ops);
    }
    return res;
}

int
net_iocp_destroy(struct iocp_t *iocp)
{
    if (iocp != NULL) {
        if (iocp->worker_num) {
            DWORD i;
            struct context_t *con;
            struct hash_iterator_t *itr;

            for (hash_begin(iocp->contexts, &itr);
                    itr != NULL;
                    hash_next(iocp->contexts, &itr)) {
                assert(itr);
                hash_val(itr, (void **)&con);
                assert(con);
                if (con->active) {
                    net_close(&(con->sock));
                }
            }
            for (i = 0; i < iocp->worker_num; ++i) {
                PostQueuedCompletionStatus(iocp->cp, 0, NULL, NULL);
            }
            WaitForMultipleObjects((DWORD)(iocp->worker_num), iocp->workers,
                                   TRUE, INFINITE);
            pool_free(iocp->workers);
        }
        CloseHandle(iocp->cp);
        hash_destroy(iocp->contexts);
        pool_free(iocp);
    }
    return 0;
}

int
net_iocp_context(struct context_t **con, struct iocp_t *iocp,
                 SOCKET s)
{
    assert(iocp);
    *con = (struct context_t *)hash_find_n(iocp->contexts, (int)s);
    return 0;
}

int
net_iocp_associate(struct iocp_t *iocp, SOCKET s, int id)
{
    int res = 0;
    struct context_t *con;
    struct hash_iterator_t *itr, *old;

    assert(iocp);
    hash_begin(iocp->contexts, &itr);
    while (itr != NULL) {
        assert(itr);
        hash_val(itr, (void **)&con);
        assert(con);
        old = itr;
        hash_next(iocp->contexts, &itr);
        if (!con->active) {
            hash_remove_i(iocp->contexts, old);
        }
    }

    net_context_create(&con, s, id);
    if (!CreateIoCompletionPort((HANDLE)s, iocp->cp, (ULONG_PTR)con, 0)) {
        LOG_ERROR("pub.net", "CreateIoCompletionPort failed: %u.",
                  GetLastError());
        net_context_destroy(con);
        res = NET_IOCPNOTASSOCIATED;
    } else {
        int i = 0;
        struct overlapped_t *ol;

        hash_insert_n(iocp->contexts, (int)s, con);
        net_overlapped_create(&ol, NIT_INIT);
        res = net_iocp_post(iocp, 0, con, ol);
    }
    return res;
}

int
net_iocp_send(struct iocp_t *iocp, SOCKET s, struct message_t *msg)
{
    int res = 0;
    struct context_t *con;

    assert(iocp && msg);
    net_message_write(msg);
    if (s == INVALID_SOCKET) {
        struct hash_iterator_t *itr;

        net_message_get_ref(msg, hash_len(iocp->contexts));
        for (hash_begin(iocp->contexts, &itr);
                itr != NULL;
                hash_next(iocp->contexts, &itr)) {
            assert(itr);
            hash_val(itr, (void **)&con);
            assert(con);
            if ((res = net_context_send(iocp, con, msg)) != 0) {
                break;
            }
        }
    } else {
        con = (struct context_t *)hash_find_n(iocp->contexts, (int)s);
        res = net_context_send(iocp, con, msg);
    }
    return res;
}

int
net_iocp_recv(SOCKET s)
{
    int res = 0;
    return res;
}

int
net_iocp_disassociate(struct iocp_t *iocp, struct context_t *con)
{
    assert(iocp && con);
    if (con->active) {
        atomic_xchg((volatile int *) & (con->active), false);
        if (con->sock != INVALID_SOCKET) {
            net_callback(con->sock, NIT_PCLS, 0, &(con->id));
        }
    }
    return 0;
}

int
net_iocp_post(struct iocp_t *iocp, DWORD size, struct context_t *con,
              struct overlapped_t *ol)
{
    int res = 0;

    assert(iocp && con && ol);
    if (!PostQueuedCompletionStatus(iocp->cp, size, (ULONG_PTR)con,
                                    &ol->overlapped) && GetLastError() != ERROR_IO_PENDING) {
        LOG_ERROR("pub.net", "PostQueuedCompletionStatus(%d) failed: %d.",
                  ol->type, GetLastError());
        net_overlapped_destroy(ol);
        res = NET_IOCPNOTPOST;
    }
    return res;
}

int
net_context_create(struct context_t **con, SOCKET s, int id)
{
    *con = (struct context_t *)pool_alloc(sizeof(**con));
    (*con)->active = true;
    (*con)->pending_send = 0;
    (*con)->pos_rd = 0;
    (*con)->msg = NULL;
    (*con)->id = id;
    (*con)->sock = s;
    (*con)->send_bufs = xqueue_create(CHUNK_COUNT);
    (*con)->recv_msgs = xqueue_create(CHUNK_COUNT);
    return 0;
}

int
net_context_destroy(struct context_t *con)
{
    if (con != NULL) {
        size_t sz;
        struct overlapped_t *ol;
        struct message_t *msg;

        while (xqueue_pop(con->recv_msgs, (void **)&msg) == 0) {
            net_message_destroy(msg);
        }
        xqueue_destroy(con->recv_msgs);
        while (xqueue_pop(con->send_bufs, (void **)&ol) == 0) {
            net_message_get_size(ol->msg, &sz);
            atomic_sub((int *)&con->pending_send, (int)sz);
            net_overlapped_destroy(ol);
        }
        if (con->pending_send != 0) {
            LOG_WARN("pub.net", "Pending send size: %u.",
                     con->pending_send);
        }
        xqueue_destroy(con->send_bufs);
        if (con->msg != NULL) {
            net_message_destroy(con->msg);
        }
        pool_free(con);
    }
    return 0;
}

int
net_context_send(struct iocp_t *iocp,
                 struct context_t *con,
                 struct message_t *msg)
{
    int res = 0;
    struct overlapped_t *ol;

    if (con == NULL || !(con->active)) {
        res = NET_CONTEXTNOTFOUND;
    } else if (con->pending_send > MAX_PENDING) {
        res = NET_IOCPSENDPENDING;
    } else {
        size_t sz;

        net_overlapped_create(&ol, NIT_SEND);
        ol->msg = msg;
        net_message_buffers(msg, &(ol->bufs), (size_t *) & (ol->count));
        net_message_get_size(msg, &sz);
        atomic_add((int *)&con->pending_send, (int)sz);
        if (con->pending_send > sz) {
            xqueue_push(con->send_bufs, ol);
        } else {
            if ((res = net_iocp_post(iocp, 0, con, ol)) != 0) {
                net_overlapped_destroy(ol);
                atomic_sub((int *)&con->pending_send, (int)sz);
            }
        }
    }
    return res;
}

int
net_iocp_onhtbk(struct iocp_t *iocp, DWORD size,
                struct context_t *con, struct overlapped_t *ol)
{
    return net_overlapped_destroy(ol);
}

int
net_iocp_oninit(struct iocp_t *iocp, DWORD size,
                struct context_t *con, struct overlapped_t *ol)
{
    net_overlapped_set(ol, NIT_RECV);
    return net_iocp_post(iocp, 0, con, ol);
}

int
net_iocp_onsend(struct iocp_t *iocp, DWORD size,
                struct context_t *con, struct overlapped_t *ol)
{
    int err, res = 0;

    assert(con && ol);
    net_overlapped_set(ol, NIT_SNDF);
    err = WSASend(con->sock, ol->bufs, (DWORD)(ol->count), &size, 0,
                  (LPWSAOVERLAPPED)ol, NULL);
    if (err == SOCKET_ERROR) {
        err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            LOG_ERROR("pub.net", "WSASend(%d) failed: %d %s.",
                      con->sock,
                      err, net_errstr(err));
            net_overlapped_destroy(ol);
            net_iocp_disassociate(iocp, con);
            atomic_sub((int *)&con->pending_send, (int)size);
            res = NET_IOCPNOTSENT;
        }
    }
    return res;
}

int
net_iocp_onrecv(struct iocp_t *iocp, DWORD size,
                struct context_t *con, struct overlapped_t *ol)
{
    int res = 0, err;
    DWORD flag = 0;

    assert(con && ol);
    net_overlapped_set(ol, NIT_RCVF);
    err = WSARecv(con->sock, ol->bufs, (DWORD)ol->count, &size, &flag,
                  &(ol->overlapped), NULL);
    if (err == SOCKET_ERROR) {
        err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            LOG_ERROR("pub.net", "WSARecv(%d) failed: %d %s.",
                      con->sock,
                      err, net_errstr(err));
            net_overlapped_destroy(ol);
            net_iocp_disassociate(iocp, con);
            res = NET_IOCPNOTRECV;
        }
    }
    return res;
}

int
net_iocp_onsndf(struct iocp_t *iocp, DWORD size,
                struct context_t *con, struct overlapped_t *ol)
{
    int res = 0;

    atomic_sub((int *)&con->pending_send, (int)size);
    net_callback(con->sock, NIT_SNDF, 0, ol->msg);
    net_overlapped_destroy(ol);
    if (xqueue_pop(con->send_bufs, (void **)&ol) == 0) {
        res = net_iocp_onsend(iocp, size, con, ol);
    }
    return res;
}

int
net_iocp_onrcvf(struct iocp_t *iocp, DWORD size,
                struct context_t *con, struct overlapped_t *ol)
{
    size_t rem, msg_size;
    char *cur_ptr;

    assert(iocp && con && ol);
    rem = (size_t)size;
    cur_ptr = ol->bufs[0].buf;
    if (size == 0) { /* graceful closure */
        net_overlapped_destroy(ol);
        net_iocp_disassociate(iocp, con);
    } else {
        net_callback(con->sock, NIT_RECV, 0, &size);
        while (rem > 0) { /* remain size */
            if (con->msg == NULL) { /* fresh message */
                con->pos_rd = 0;
                con->msg = net_message_create(-1);
            }
            net_message_append_buf(con->msg, &cur_ptr, &rem, &(con->pos_rd));
            net_message_get_size(con->msg, &msg_size);
            if (msg_size == con->pos_rd) { /* completed message */
                net_callback(con->sock, NIT_RCVF, 0, con->msg);
                con->msg = NULL;
            }
        }
        net_iocp_onrecv(iocp, CHUNK_SIZE, con, ol);
    }
    return 0;
}

int
net_overlapped_create(struct overlapped_t **ol, io_type type)
{
    *ol = (struct overlapped_t *)pool_alloc(sizeof(**ol));
    ZeroMemory(&((*ol)->overlapped), sizeof((*ol)->overlapped));
    net_overlapped_set(*ol, type);
    (*ol)->msg = NULL;
    (*ol)->sn = -1;
    switch (type) {
    case NIT_INIT:
        (*ol)->bufs = (LPWSABUF)pool_alloc(sizeof(*((*ol)->bufs)));
        (*ol)->bufs->buf = (char *)pool_alloc(CHUNK_SIZE);
        (*ol)->bufs->len = CHUNK_SIZE;
        (*ol)->count = 1;
        break;
    case NIT_SEND:
        (*ol)->bufs = NULL;
        (*ol)->count = 0;
        break;
    default:
        LOG_ERROR("pub.net", "Invalid I/O type: %d.", type);
    }
    return 0;
}

int
net_overlapped_destroy(struct overlapped_t *ol)
{
    size_t i;

    assert(ol);
    if (ol->bufs != NULL) {
        if (ol->type == NIT_RCVF) {
            for (i = 0; i < ol->count; ++i) {
                pool_free(ol->bufs[i].buf);
            }
        }
        pool_free(ol->bufs);
    }
    if ((ol->type == NIT_SEND || ol->type == NIT_SNDF) && ol->msg != NULL) {
        net_message_destroy(ol->msg);
    }
    pool_free(ol);
    return 0;
}

int
net_overlapped_set(struct overlapped_t *ol, io_type type)
{
    assert(ol);
    ol->type = type;
    switch (type) {
    case NIT_INIT:
        ol->func = net_iocp_oninit;
        break;
    case NIT_RECV:
        ol->func = net_iocp_onrecv;
        break;
    case NIT_RCVF:
        ol->func = net_iocp_onrcvf;
        break;
    case NIT_SEND:
        ol->func = net_iocp_onsend;
        break;
    case NIT_SNDF:
        ol->func = net_iocp_onsndf;
        break;
    default:
        LOG_ERROR("pub.net", "Invalid I/O type: %d.", type);
    }
    return 0;
}

unsigned int __stdcall
net_iocp_worker(void *args)
{
    BOOL status;
    DWORD size;
    DWORD err;
    struct context_t *con;
    struct overlapped_t *ol;
    struct iocp_t *iocp;

    assert(args);
    iocp = (struct iocp_t *)args;
    while (true) {
        status = GetQueuedCompletionStatus(iocp->cp, &size, (PULONG_PTR)&con,
                                           (LPOVERLAPPED *)&ol, INFINITE);
        if (!status) {
            err = GetLastError();
            if (ol == NULL) {
                LOG_ERROR("pub.net", "GetQueuedCompletionStatus failed: %u.",
                          err);
            } else if (size == 0) { /* socket closed */
                net_overlapped_destroy(ol);
                net_iocp_disassociate(iocp, con);
            } else {
                net_overlapped_destroy(ol);
                LOG_ERROR("pub.net", "GetQueuedCompletionStatus failed: %u",
                          err);
            }
        } else if (ol == NULL) { /* thread exits */
            break;
        } else {
            ol->func(iocp, size, con, ol);
        }
    }
    return 0;
}
