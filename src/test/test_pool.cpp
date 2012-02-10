#include "public/public.h"
#include <Winsock2.h>
#include "public/pool/chunk.h"
#include "vld/vld.h"

int main(void)
{
    void *b;
    struct chunk_t *c = NULL;
    LPWSABUF bufs;
    size_t count;
    const char *str =
        "09/21/11 14:33:40.566 [2500] TRACE [0x0001]\n"
        "09/21/11 14:33:40.581 [5108] TRACE [0x0001]\n"
        "09/21/11 14:33:40.597 [1200] TRACE [0x0001]\n"
        "09/21/11 14:33:40.612 [3176] TRACE [0x0001]\n"
        "09/21/11 14:33:40.612 [4260] TRACE [0x0100]\n"
        "09/21/11 14:33:40.628 [5368] TRACE [0x0100]\n"
        "09/21/11 14:33:40.644 [3376] TRACE [0x0100]\n"
        "09/21/11 14:33:40.659 [4248] TRACE [0x0100]\n"
        "09/21/11 14:33:40.675 [3844] TRACE [0x0100]\n";

    pool_init();
    b = pool_alloc(50);
    c = chunk_create(0);
    chunk_append(c, "hello, world", 12);
    chunk_append(c, (void *)str, strlen(str));
    chunk_retrive(c, b, 12);
    LOG_INFO("test.pool", "chunk : %s.", b);
    chunk_buffers(c, &bufs, &count);
    pool_free(bufs);
    chunk_destroy(c);
    pool_free(b);
    pool_fini();
    return 0;
}
