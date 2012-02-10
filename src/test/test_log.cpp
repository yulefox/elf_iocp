#include "public/public.h"

#define MAX_LOG 1000
#pragma comment (lib, "Winmm.lib")

int main(void)
{
    log_init();
    DWORD st, et;
    int i;
    //LOG_FATAL("test.log", "hello");
    //LOG_ERROR("test.log", "hello, %s", "world");
    //LOG_WARN("test.log", "hello");
    //LOG_INFO("test.log", "hello");
    //LOG_DEBUG("test.log", "hello");
    st = timeGetTime();
    for (i = 0; i < 2000000000; ++i)
    {
    }

    et = timeGetTime();
    LOG_INFO("test.log", "SPEND TIME: %u; %u.",
        (et - st), i);
    log_fini();
    return 0;
}
