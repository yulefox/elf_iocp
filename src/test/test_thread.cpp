#include "public/public.h"
#include "public/thread/thread.h"
#include "vld/vld.h"

volatile bool running = true;

static unsigned int __stdcall read(void *args)
{
    while (!thread_isexit(args)) {
        LOG_INFO("test.thread", "Read thread is running...");
    }
    return 0;
}

static unsigned int __stdcall write(void *args)
{
    while (!thread_isexit(args)) {
        LOG_INFO("test.thread", "Write thread is running...");
    }
    return 0;
}

static BOOL ctrl_handler(DWORD type)
{
    BOOL res = TRUE;

    switch (type) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
        LOG_INFO("test.net", "Process exits.");
        running = false;
        break;
    default:
        LOG_WARN("test.net", "UNKNOWN_EVENT (%d).", type);
        res = FALSE;
    }
    return res;
}

int main(void)
{
    struct thread_t *tr, *tw;
    mutex_t l;

    pool_init();
    mutex_init(&l);
    mutex_lock(&l);
    mutex_unlock(&l);
    mutex_init(&l);
    thread_create(&tr, read, NULL);
    thread_create(&tw, write, NULL);
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE)) {
        LOG_INFO("test.net", "Console control handler is installed.");
        while (running) {
            Sleep(100);
        }
    }
    thread_destroy(tw);
    thread_destroy(tr);
    pool_fini();
    return 0;
}
