#include "public.h"

int public_init(void)
{
    MODULE_IMPORT_SWITCH;
    return pool_init();
}

int public_fini(void)
{
    MODULE_IMPORT_SWITCH;
    return pool_fini();
}
