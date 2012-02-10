#include "public/net.h"
#include "public/public.h"
#include "public/thread/thread.h"
#include "vld/vld.h"

#pragma comment (lib, "Winmm.lib")

volatile bool running = true;

static unsigned int __stdcall recv(void *args);
static unsigned int __stdcall send(void *args);

unsigned int __stdcall recv(void *args)
{
    return 0;
}

unsigned int __stdcall send(void *args)
{
    int a = 0;
    DWORD st, et;
    struct message_t *msg;
    char *str = "hello, world\n"
        "Console control handler is installed."
        "Console control handler is installed."
        "Console control handler is installed."
        "Console control handler is installed."
        "Console control handler is installed."
        "Console control handler is installed."
        "Console control handler is installed."
        "Console control handler is installed."
        "Console control handler is installed.";

    LOG_INFO("test.net.send", "Send thread is running...");
    st = timeGetTime();
    while (!thread_isexit(args)) {
        msg = net_message_create(1);
        net_message_append_int(msg, 50);
        net_message_append_str(msg, str);
        if (net_send(-1, msg)) {
            et = timeGetTime();
            if (et == st) et = st + 1;
            LOG_INFO("test.net.send", "Send thread sent %d items (%d/s).", a,
                a * 1000 / (et - st));
            Sleep(10);
            st = et;
            a = 0;
        } else {
            ++a;
        }
    }
    et = timeGetTime();
    if (et == st) et = st + 1;
    LOG_INFO("test.net.send", "Send thread sent %d items (%d/s).", a,
        a * 1000 / (et - st));
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
    struct thread_t *r;

    net_init();
    LOG_INFO("test.net", "Net module initilized.");
    thread_create(&r, recv, NULL);
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE)) {
        LOG_INFO("test.net", "Console control handler is installed.");
        while (running) {
            Sleep(100);
        }
    }
    thread_destroy(r);
    net_fini();
    return 0;
}
