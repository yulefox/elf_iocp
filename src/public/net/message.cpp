#include "message.h"
#include "public/pool/chunk.h"

#define BLOCK_SIZE  1024
#define MAX_STRING  4096
#define MSG_HEAD_SIZE sizeof(struct message_head_t)

struct message_head_t {
    size_t pad;
    size_t size;
    int type;
    int code;
};

struct message_t {
    struct chunk_t *chunk;
    struct message_head_t head;
    size_t ref;
};

struct message_t *
    net_message_create(int type)
{
    message_t *msg;
    msg = (message_t *)pool_alloc(sizeof(*msg));
    msg->head.type = type;
    msg->head.pad = 0;
    msg->head.size = 0;
    msg->ref = 1;
    msg->chunk = chunk_create(0);
    if (type != -1) {
        net_message_append_raw(msg, &(msg->head), MSG_HEAD_SIZE);
    }
    return msg;
}

int
net_message_destroy(struct message_t *msg)
{
    if (msg != NULL && --(msg->ref) == 0) {
        chunk_destroy(msg->chunk);
        pool_free(msg);
    }
    return 0;
}

bool
net_message_complete(struct message_t *msg)
{
    assert(msg);
    return false;
}

int
net_message_read(struct message_t *msg)
{
    assert(msg);
    return net_message_retrive_raw(msg, &msg->head, MSG_HEAD_SIZE);
}

int
net_message_write(struct message_t *msg)
{
    assert(msg);
    msg->head.size = msg->head.pad - 4;
    return chunk_set(msg->chunk, &msg->head, MSG_HEAD_SIZE);
}

int
net_message_get_size(struct message_t *msg, size_t *size)
{
    assert(msg);
    *size = msg->head.pad;
    return 0;
}

int
net_message_get_type(struct message_t *msg, int *type)
{
    assert(msg);
    *type = msg->head.type;
    return 0;
}

int
net_message_set_type(struct message_t *msg, int type)
{
    assert(msg);
    msg->head.type = type;
    return 0;
}

int
net_message_get_ref(struct message_t *msg, size_t num)
{
    assert(msg);
    msg->ref = num;
    return 0;
}

int
net_message_buffers(struct message_t *msg, struct _WSABUF **bufs,
                        size_t *count)
{
    return chunk_buffers(msg->chunk, bufs, count);
}

int
net_message_append_int(struct message_t *msg, int val)
{
    net_message_append_raw(msg, &val, sizeof(val));
    return 0;
}

int
net_message_append_int64(struct message_t *msg, __int64 val)
{
    net_message_append_raw(msg, &val, sizeof(val));
    return 0;
}

int
net_message_append_size(struct message_t *msg, size_t val)
{
    net_message_append_raw(msg, &val, sizeof(val));
    return 0;
}

int
net_message_append_float(struct message_t *msg, float val)
{
    net_message_append_raw(msg, &val, sizeof(val));
    return 0;
}

int
net_message_append_str(struct message_t *msg, const char *val)
{
    net_message_append_size(msg, strlen(val));
    net_message_append_raw(msg, (void *)val, strlen(val));
    return 0;
}

size_t
net_message_append_raw(struct message_t *msg, void *val, size_t size)
{
    assert(msg && val);
    msg->head.pad += size;
    return chunk_append(msg->chunk, val, size);
}

size_t
net_message_append_buf(struct message_t *msg, char **buf, size_t *size,
                       size_t *pos)
{
    size_t rd, cur;

    assert(msg && buf);
    if (*pos < MSG_HEAD_SIZE) {
        rd = min(MSG_HEAD_SIZE - *pos, *size);
        cur = *pos + rd;
        chunk_append(msg->chunk, *buf, rd);
        *buf += rd;
        *size -= rd;
        if (cur >= MSG_HEAD_SIZE) {
            net_message_read(msg);
            *pos = cur;
        }
    }
    if (*size > 0 && *pos >= MSG_HEAD_SIZE) {
        rd = min(msg->head.pad - *pos, *size);
        chunk_append(msg->chunk, *buf, rd);
        *buf += rd;
        *pos += rd;
        *size -= rd;
    }
    return 0;
}

int
net_message_retrive_int(struct message_t *msg)
{
    int val;

    net_message_retrive_raw(msg, &val, sizeof(val));
    return val;
}

__int64
net_message_retrive_int64(struct message_t *msg)
{
    __int64 val;

    net_message_retrive_raw(msg, &val, sizeof(val));
    return val;
}

size_t
net_message_retrive_size(struct message_t *msg)
{
    size_t val;

    net_message_retrive_raw(msg, &val, sizeof(val));
    return val;
}

float
net_message_retrive_float(struct message_t *msg)
{
    float val;

    net_message_retrive_raw(msg, &val, sizeof(float));
    return val;
}

const char *
net_message_retrive_str(struct message_t *msg, char *val)
{
    size_t len;

    len = net_message_retrive_size(msg);
    assert(len < MAX_STRING);
    net_message_retrive_raw(msg, val, len);
    val[len] = '\0';
    return val;
}

int
net_message_retrive_raw(struct message_t *msg, void *buf, size_t size)
{
    assert(msg && buf);
    chunk_retrive(msg->chunk, buf, size);
    return 0;
}
