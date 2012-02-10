#include "pool.h"
#include <stdlib.h>
#include "public/log/log.h"

int pool_init(void)
{
    MODULE_IMPORT_SWITCH;
    return log_init();
}

int pool_fini(void)
{
    MODULE_IMPORT_SWITCH;
    return log_fini();
}

void *pool_alloc(size_t size)
{
    register void *buf;

    buf = (size > 0) ? malloc(size) : NULL;
    assert(buf);
    return buf;
}

void pool_free(void *block)
{
    if (block != NULL)
        free(block);
}
