#include "public/public.h"
#include "public/xqueue.h"
#include "public/thread/thread.h"
#include "vld/vld.h"

#pragma comment (lib, "Winmm.lib")

volatile bool running = true;

static unsigned int __stdcall write(void *args)
{
    struct xqueue_t *q;
    int a = 0;
    DWORD st, et;

    LOG_INFO("test.xqueue.write", "Write thread is running...");
    st = timeGetTime();
    q = (struct xqueue_t *)thread_args((struct thread_t *)args);
    while (!thread_isexit((struct thread_t *)args)) {
        xqueue_push(q, NULL);
        ++a;
   }
    et = timeGetTime();
    LOG_INFO("test.xqueue.read", "Read thread pushes %d items (%.2f/s).", a,
        a * 1.0 / (et - st) * 1000);
    return 0;
}

static unsigned int __stdcall read(void *args)
{
    struct xqueue_t *q;
    void *msg;
    int a = 0;
    DWORD st, et;

    LOG_INFO("test.xqueue.read", "Read thread is running...");
    st = timeGetTime();
    q = (struct xqueue_t *)thread_args((struct thread_t *)args);
    while (!thread_isexit((struct thread_t *)args)) {
        if (xqueue_pop(q, &msg) == 0) {
            ++a;
        } else {
        }
    }
    et = timeGetTime();
    LOG_INFO("test.xqueue.read", "Read thread pop %d items (%.2f/s).", a,
        a * 1.0 / (et - st) * 1000);
    return 0;
}

static BOOL ctrl_handler(DWORD type)
{
    BOOL res = TRUE;

    switch (type) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
        LOG_INFO("test.xqueue", "Process exits.");
        running = false;
        break;
    default:
        LOG_WARN("test.xqueue", "UNKNOWN_EVENT (%d).", type);
        res = FALSE;
    }
    return res;
}

int main(void)
{
    struct thread_t *tw, *tr;
    struct xqueue_t *q;

    pool_init();
    q = xqueue_create(16);
    thread_create(&tr, read, q);
    thread_create(&tw, write, q);
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE)) {
        LOG_INFO("test.xqueue", "Console control handler is installed.");
        while (running) {
            Sleep(100);
        }
    }
    thread_destroy(tw);
    thread_destroy(tr);
    xqueue_destroy(q);
    pool_fini();
    return 0;
}
